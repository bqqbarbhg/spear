#include "Json.h"
#include "sf/Reflection.h"

namespace sp {

static void writeJsonFieldsImp(jso_stream &dst, char *base, sf::Type *type)
{
	if (type->baseType) writeJsonFieldsImp(dst, base, type->baseType);
	for (const sf::Field &field : type->fields) {
		jso_prop_len(&dst, field.name.data, field.name.size);
		if (dst.pretty && field.flags & sf::Field::CompactString) jso_single_line(&dst);
		writeInstJson(dst, base + field.offset, field.type, type);
	}
}

void writeInstJson(jso_stream &dst, void *inst, sf::Type *type, sf::Type *parentType)
{
	uint32_t flags = type->flags;
	char *base = (char*)inst;

#if 0
	if (!parentType || !((parentType->flags & sf::Type::HasArray) && (type->flags & sf::Type::IsPrimitive))) {
		sf::SmallStringBuf<128> name;
		name.append("/* ");
		type->getName(name);
		name.append(" */ ");
		jso_raw_append_len(&dst, name.data, name.size);
	}
#endif

	if (flags & sf::Type::HasString) {
		sf::VoidSlice slice = type->instGetArray(inst);
		jso_string_len(&dst, (char*)slice.data, slice.size);
	} else if (flags & sf::Type::Polymorph) {
		if (dst.pretty && flags & sf::Type::CompactString) jso_single_line(&dst);

		sf::PolymorphInstance poly = type->instGetPolymorph(inst);

		if (poly.type) {
			jso_object(&dst);
			sf::CString tagName = type->getPolymorphTagName();
			jso_prop_len(&dst, tagName.data, tagName.size);
			jso_string_len(&dst, poly.type->name.data, poly.type->name.size);

			char *polyBase = (char*)poly.inst;
			writeJsonFieldsImp(dst, polyBase, poly.type->type);

			jso_end_object(&dst);
		} else {
			jso_null(&dst);
		}

	} else if (flags & sf::Type::HasFields) {
		if (dst.pretty && flags & sf::Type::CompactString) jso_single_line(&dst);
		jso_object(&dst);
		writeJsonFieldsImp(dst, base, type);
		jso_end_object(&dst);
	} else if (flags & sf::Type::HasPointer) {
		void *ptr = type->instGetPointer(inst);
		if (ptr) {
			writeInstJson(dst, ptr, type->elementType, type);
		} else {
			jso_null(&dst);
		}
	} else if (flags & sf::Type::HasArray) {
		if (dst.pretty && flags & sf::Type::CompactString) jso_single_line(&dst);
		sf::Type *elem = type->elementType;
		size_t elemSize = elem->info.size;
		sf::VoidSlice slice = type->instGetArray(inst);
		uint32_t size = (uint32_t)slice.size;

#if 0
		{
			sf::SmallStringBuf<128> comment;
			comment.format("/* size=%u */ ", size);
			jso_raw_append_len(&dst, comment.data, comment.size);
		}
#endif

		jso_array(&dst);
		char *ptr = (char*)slice.data;
		for (uint32_t i = 0; i < size; i++) {
			writeInstJson(dst, ptr, elem, type);
			ptr += elemSize;
		}
		jso_end_array(&dst);
	} else if (flags & sf::Type::IsPrimitive) {
		switch (type->primitive) {
		case sf::Type::Bool: jso_boolean(&dst, *(bool*)inst); break;
		case sf::Type::Char: jso_uint(&dst, (unsigned)(uint8_t)*(char*)inst); break;
		case sf::Type::I8: jso_int(&dst, *(int8_t*)inst); break;
		case sf::Type::I16: jso_int(&dst, *(int16_t*)inst); break;
		case sf::Type::I32: jso_int(&dst, *(int32_t*)inst); break;
		case sf::Type::I64: jso_int64(&dst, *(int64_t*)inst); break;
		case sf::Type::U8: jso_uint(&dst, *(uint8_t*)inst); break;
		case sf::Type::U16: jso_uint(&dst, *(uint16_t*)inst); break;
		case sf::Type::U32: jso_uint(&dst, *(uint32_t*)inst); break;
		case sf::Type::U64: jso_uint64(&dst, *(uint64_t*)inst); break;
		case sf::Type::F32: jso_double(&dst, *(float*)inst); break;
		case sf::Type::F64: jso_double(&dst, *(double*)inst); break;
		}
	} else {
		// TODO: Binary serialization
		jso_string(&dst, "");
	}
}

static bool readJsonFieldsImp(jsi_value *src, char *base, sf::Type *type)
{
	if (type->baseType) {
		if (!readJsonFieldsImp(src, base, type->baseType)) return false;
	}
	for (const sf::Field &field : type->fields) {
		jsi_value *child = jsi_get_len(src->object, field.name.data, field.name.size);
		if (!child) continue;
		if (!readInstJson(child, base + field.offset, field.type)) return false;
	}
	return true;
}

bool readInstJson(jsi_value *src, void *inst, sf::Type *type)
{
	uint32_t flags = type->flags;
	char *base = (char*)inst;
	if (flags & sf::Type::HasSetString && src->type == jsi_type_string) {
		if (src->flags & jsi_flag_multiline) {
			sf::SmallStringBuf<1024> buf;
			const char *ptr = src->string;
			if (*ptr == '\r') ptr++;
			if (*ptr == '\n') ptr++;
			uint32_t target_spaces = 0, target_tabs = 0;
			for (;; ptr++) {
				if (*ptr == ' ') target_spaces++;
				else if (*ptr == '\t') target_tabs++;
				else break;
			}

			for (; *ptr != '\r' && *ptr != '\n' && *ptr != '\0'; ptr++) {
				buf.append(*ptr);
			}

			while (*ptr != '\0') {
				if (*ptr == '\r') ptr++;
				if (*ptr == '\n') ptr++;
				uint32_t spaces = 0, tabs = 0;
				for (; spaces < target_spaces || tabs < target_tabs; ptr++) {
					if (*ptr == ' ') spaces++;
					else if (*ptr == '\t') tabs++;
					else break;
				}
				if (*ptr == '\0') break;
				buf.append('\n');
				for (; *ptr != '\r' && *ptr != '\n' && *ptr != '\0'; ptr++) {
					buf.append(*ptr);
				}
			}

			type->instSetString(inst, buf);
		} else {
			size_t len = jsi_length(src->string);
			type->instSetString(inst, sf::String(src->string, len));
		}
	} else if ((flags & (sf::Type::HasString | sf::Type::HasArrayResize)) == (sf::Type::HasString | sf::Type::HasArrayResize)) {
		if (src->type == jsi_type_string) {
			size_t len = jsi_length(src->string);
			sf::VoidSlice slice = type->instArrayReserve(inst, len);
			memcpy(slice.data, src->string, len);
			type->instArrayResize(inst, len);
		} else {
			return false;
		}
	} else if (flags & sf::Type::Polymorph) {
		if (src->type == jsi_type_object) {
			jsi_value *tag = jsi_get_len(src->object, "type", 4);
			jsi_value *data = jsi_get_len(src->object, "data", 4);
			if (tag->type != jsi_type_string) return false;

			sf::String name { tag->string, jsi_length(tag->string) };
			const sf::PolymorphType *poly = type->elementType->getPolymorphTypeByName(name);
			char *polyBase = (char*)type->instSetPolymorph(inst, poly->type);
			if (!readJsonFieldsImp(src, polyBase, poly->type)) return false;

		} else if (src->type != jsi_type_null) {
			return false;
		}
	} else if (flags & sf::Type::HasFields) {
		if (src->type == jsi_type_object) {
			if (!readJsonFieldsImp(src, base, type)) return false;
		} else {
			return false;
		}
	} else if (flags & sf::Type::HasPointer) {
		if (src->type != jsi_type_null) {
			void *ptr = type->instSetPointer(inst);
			readInstJson(src, ptr, type->elementType);
		}
	} else if (flags & sf::Type::HasArrayResize) {
		if (src->type == jsi_type_array) {
			sf::Type *elem = type->elementType;
			size_t elemSize = elem->info.size;
			uint32_t size = (uint32_t)src->array->num_values;

			sf::VoidSlice slice = type->instArrayReserve(inst, size);
			char *ptr = (char*)slice.data;
			jsi_value *val = src->array->values;
			for (uint32_t i = 0; i < size; i++) {
				if (!readInstJson(val, ptr, elem)) return false;
				ptr += elemSize;
				val++;
			}

			type->instArrayResize(inst, size);
		} else {
			return false;
		}
	} else if (flags & sf::Type::IsPrimitive) {
		if (src->type == jsi_type_number) {
			if (src->flags & jsi_flag_stored_as_int64) {
				int64_t num = src->int64_storage;
				switch (type->primitive) {
				case sf::Type::Bool: *(bool*)inst = (bool)num; break;
				case sf::Type::Char: *(char*)inst = (char)(uint8_t)num; break;
				case sf::Type::I8: *(int8_t*)inst = (int8_t)num; break;
				case sf::Type::I16: *(int16_t*)inst = (int16_t)num; break;
				case sf::Type::I32: *(int32_t*)inst = (int32_t)num; break;
				case sf::Type::I64: *(int64_t*)inst = (int64_t)num; break;
				case sf::Type::U8: *(uint8_t*)inst = (uint8_t)num; break;
				case sf::Type::U16: *(uint16_t*)inst = (uint16_t)num; break;
				case sf::Type::U32: *(uint32_t*)inst = (uint32_t)num; break;
				case sf::Type::U64: *(uint64_t*)inst = (uint64_t)num; break;
				case sf::Type::F32: *(float*)inst = (float)num; break;
				case sf::Type::F64: *(double*)inst = (double)num; break;
				}
			} else {
				double num = src->number;
				switch (type->primitive) {
				case sf::Type::Bool: *(bool*)inst = (bool)num; break;
				case sf::Type::Char: *(char*)inst = (char)(uint8_t)num; break;
				case sf::Type::I8: *(int8_t*)inst = (int8_t)num; break;
				case sf::Type::I16: *(int16_t*)inst = (int16_t)num; break;
				case sf::Type::I32: *(int32_t*)inst = (int32_t)num; break;
				case sf::Type::I64: *(int64_t*)inst = (int64_t)num; break;
				case sf::Type::U8: *(uint8_t*)inst = (uint8_t)num; break;
				case sf::Type::U16: *(uint16_t*)inst = (uint16_t)num; break;
				case sf::Type::U32: *(uint32_t*)inst = (uint32_t)num; break;
				case sf::Type::U64: *(uint64_t*)inst = (uint64_t)num; break;
				case sf::Type::F32: *(float*)inst = (float)num; break;
				case sf::Type::F64: *(double*)inst = (double)num; break;
				}
			}
		} else if (src->type == jsi_type_boolean) {
			switch (type->primitive) {
			case sf::Type::Bool: *(bool*)inst = src->boolean; break;
			default: return false;
			}
		} else {
			// TODO: String
			return false;
		}
	} else {
		// TODO: Binary serialization
	}

	if (type->postSerializeFn) {
		type->postSerializeFn(inst, type);
	}

	return true;
}

}

