#include "Reflection.h"
#include "Vector.h"
#include "ext/mx/mx_platform.h"

namespace sf {

void Type::getName(sf::StringBuf &buf)
{
	buf.append(name);
}

void Type::instConstruct(void *inst, size_t num)
{
	memset(inst, 0, num * size);
}

void Type::instMove(void *dst, void *src, size_t num)
{
	memcpy(dst, src, num * size);
}

void Type::instDestruct(void *inst, size_t num)
{
}

VoidSlice Type::instGetArray(void *inst)
{
	return { };
}

VoidSlice Type::instArrayReserve(void *inst, size_t size)
{
	sf_assert(0 && "Array resize not supported");
	return { };
}

void Type::instArrayResize(void *inst, size_t size)
{
}

sf::String Type::instGetString(void *inst)
{
	return { };
}

static constexpr const uint32_t PrimitiveFlags = Type::IsPrimitive|Type::IsPod|Type::CompactString;
template<> void initType<bool>(Type *t) { new (t) Type("bool", sizeof(bool), PrimitiveFlags); t->primitive = Type::Bool; }
template<> void initType<char>(Type *t) { new (t) Type("char", sizeof(char), PrimitiveFlags); t->primitive = Type::Char; }
template<> void initType<int8_t>(Type *t) { new (t) Type("int8_t", sizeof(int8_t), PrimitiveFlags); t->primitive = Type::I8; }
template<> void initType<int16_t>(Type *t) { new (t) Type("int16_t", sizeof(int16_t), PrimitiveFlags); t->primitive = Type::I16; }
template<> void initType<int32_t>(Type *t) { new (t) Type("int32_t", sizeof(int32_t), PrimitiveFlags); t->primitive = Type::I32; }
template<> void initType<int64_t>(Type *t) { new (t) Type("int64_t", sizeof(int64_t), PrimitiveFlags); t->primitive = Type::I64; }
template<> void initType<uint8_t>(Type *t) { new (t) Type("uint8_t", sizeof(uint8_t), PrimitiveFlags); t->primitive = Type::U8; }
template<> void initType<uint16_t>(Type *t) { new (t) Type("uint16_t", sizeof(uint16_t), PrimitiveFlags); t->primitive = Type::U16; }
template<> void initType<uint32_t>(Type *t) { new (t) Type("uint32_t", sizeof(uint32_t), PrimitiveFlags); t->primitive = Type::U32; }
template<> void initType<uint64_t>(Type *t) { new (t) Type("uint64_t", sizeof(uint64_t), PrimitiveFlags); t->primitive = Type::U64; }
template<> void initType<float>(Type *t) { new (t) Type("float", sizeof(float), PrimitiveFlags); t->primitive = Type::F32; }
template<> void initType<double>(Type *t) { new (t) Type("double", sizeof(double), PrimitiveFlags); t->primitive = Type::F64; }

void writeInstBinary(sf::Array<char> &dst, void *inst, Type *type)
{
	uint32_t flags = type->flags;
	char *base = (char*)inst;
	if (flags & Type::IsPod) {
		dst.push(base, type->size);
	} else if (flags & Type::HasFields) {
		for (const Field &field : type->fields) {
			writeInstBinary(dst, base + field.offset, field.type);
		}
	} else if (flags & Type::HasArray) {
		Type *elem = type->elementType;
		size_t elemSize = elem->size;
		VoidSlice slice = type->instGetArray(inst);
		uint32_t size = (uint32_t)slice.size;
		dst.push((char*)&size, sizeof(uint32_t));
		if (elem->flags & Type::IsPod) {
			dst.push((char*)slice.data, slice.size * elemSize);
		} else {
			char *ptr = (char*)slice.data;
			for (uint32_t i = 0; i < size; i++) {
				writeInstBinary(dst, ptr, elem);
				ptr += elemSize;
			}
		}
	} else {
		// TODO: Serialization callback
		sf_assert(0 && "Cannot serialize type");
	}
}

bool readInstBinary(sf::Slice<char> &src, void *inst, Type *type)
{
	uint32_t flags = type->flags;
	char *base = (char*)inst;
	if (flags & Type::IsPod) {
		if (src.size < type->size) return false;
		memcpy(inst, src.data, type->size);
		src = src.drop(type->size);
	} else if (flags & Type::HasFields) {
		for (const Field &field : type->fields) {
			if (!readInstBinary(src, base + field.offset, field.type)) return false;
		}
	} else if (flags & Type::HasArray) {
		Type *elem = type->elementType;
		size_t elemSize = elem->size;
		uint32_t size;
		if (src.size < sizeof(uint32_t)) return false;
		memcpy(&size, src.data,	sizeof(uint32_t));
		src = src.drop(sizeof(uint32_t));

		VoidSlice slice = type->instArrayReserve(inst, size);
		if (elem->flags & Type::IsPod) {
			size_t copySize = slice.size * elemSize;
			if (src.size < copySize) return false;
			memcpy(slice.data, src.data, copySize);
			src = src.drop(copySize);
		} else {
			char *ptr = (char*)slice.data;
			for (uint32_t i = 0; i < size; i++) {
				if (!readInstBinary(src, ptr, elem)) return false;
				ptr += elemSize;
			}
		}

		type->instArrayResize(inst, size);
	} else {
		// TODO: Serialization callback
		sf_assert(0 && "Cannot serialize type");
	}

	return true;
}

}
