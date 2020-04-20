#include "Json.h"
#include "sf/Reflection.h"

namespace sp {

void writeInstJson(jso_stream &dst, void *inst, sf::Type *type)
{
	uint32_t flags = type->flags;
	char *base = (char*)inst;
	if (flags & sf::Type::HasString) {
		sf::VoidSlice slice = type->instGetArray(inst);
		jso_string_len(&dst, (char*)slice.data, slice.size);
	} else if (flags & sf::Type::HasFields) {
		if (dst.pretty && flags & sf::Type::CompactString) jso_single_line(&dst);
		jso_object(&dst);
		for (const sf::Field &field : type->fields) {
			jso_prop_len(&dst, field.name.data, field.name.size);
			writeInstJson(dst, base + field.offset, field.type);
		}
		jso_end_object(&dst);
	} else if (flags & sf::Type::HasArray) {
		if (dst.pretty && flags & sf::Type::CompactString) jso_single_line(&dst);
		jso_array(&dst);
		sf::Type *elem = type->elementType;
		size_t elemSize = elem->size;
		sf::VoidSlice slice = type->instGetArray(inst);
		uint32_t size = (uint32_t)slice.size;
		char *ptr = (char*)slice.data;
		for (uint32_t i = 0; i < size; i++) {
			writeInstJson(dst, ptr, elem);
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
		case sf::Type::I64: jso_double(&dst, (double)*(int64_t*)inst); break; // TODO: String
		case sf::Type::U8: jso_uint(&dst, *(uint8_t*)inst); break;
		case sf::Type::U16: jso_uint(&dst, *(uint16_t*)inst); break;
		case sf::Type::U32: jso_uint(&dst, *(uint32_t*)inst); break;
		case sf::Type::U64: jso_double(&dst, (double)*(uint64_t*)inst); break; // TODO: String
		case sf::Type::F32: jso_double(&dst, *(float*)inst); break;
		case sf::Type::F64: jso_double(&dst, *(double*)inst); break;
		}
	} else {
		// TODO: Binary serialization
		jso_string(&dst, "");
	}
}

bool readInstJson(jsi_value *src, void *inst, sf::Type *type)
{
	uint32_t flags = type->flags;
	char *base = (char*)inst;
	if (flags & sf::Type::HasString) {
		if (src->type == jsi_type_string) {
			size_t len = jsi_length(src->string);
			sf::VoidSlice slice = type->instArrayReserve(inst, len);
			memcpy(slice.data, src->string, len);
			type->instArrayResize(inst, len);
		} else {
			return false;
		}
	} else if (flags & sf::Type::HasFields) {
		if (src->type == jsi_type_object) {
			for (const sf::Field &field : type->fields) {
				jsi_value *child = jsi_get_len(src->object, field.name.data, field.name.size);
				if (!readInstJson(child, base + field.offset, field.type)) return false;
			}
		} else {
			return false;
		}
	} else if (flags & sf::Type::HasArray) {
		if (src->type == jsi_type_array) {
			sf::Type *elem = type->elementType;
			size_t elemSize = elem->size;
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

	return true;
}

}

