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

template <typename T>
struct PrimitiveImp : TypePrimitive {
	PrimitiveImp(const char *name, bool isSigned, bool isFloat)
		: TypePrimitive(name, sizeof(T), IsPod, isSigned, isFloat)
	{
	}
};

template<> Type *initType<char>() { static PrimitiveImp<char> t { "char", false, false }; return &t; }
template<> Type *initType<int8_t>() { static PrimitiveImp<int8_t> t { "int8_t", true, false }; return &t; }
template<> Type *initType<int16_t>() { static PrimitiveImp<int16_t> t { "int16_t", true, false }; return &t; }
template<> Type *initType<int32_t>() { static PrimitiveImp<int32_t> t { "int32_t", true, false }; return &t; }
template<> Type *initType<int64_t>() { static PrimitiveImp<int64_t> t { "int64_t", true, false }; return &t; }
template<> Type *initType<uint8_t>() { static PrimitiveImp<uint8_t> t { "uint8_t", false, false }; return &t; }
template<> Type *initType<uint16_t>() { static PrimitiveImp<uint16_t> t { "uint16_t", false, false }; return &t; }
template<> Type *initType<uint32_t>() { static PrimitiveImp<uint32_t> t { "uint32_t", false, false }; return &t; }
template<> Type *initType<uint64_t>() { static PrimitiveImp<uint64_t> t { "uint64_t", false, false }; return &t; }
template<> Type *initType<float>() { static PrimitiveImp<float> t { "float", true, true }; return &t; }
template<> Type *initType<double>() { static PrimitiveImp<double> t { "double", true, true }; return &t; }

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
	}
}

}
