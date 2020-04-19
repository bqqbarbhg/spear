#include "Json.h"
#include "sf/Reflection.h"

namespace sp {

void writeInstJson(jso_stream &dst, void *inst, sf::Type *type)
{
	uint32_t flags = type->flags;
	char *base = (char*)inst;
	if (flags & sf::Type::HasString) {
		sf::String str = type->instGetString(inst);
		jso_string_len(&dst, str.data, str.size);
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
	return true;
}

}

