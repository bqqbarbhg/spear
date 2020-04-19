#include "Reflection.h"
#include "Vector.h"
#include "ext/mx/mx_platform.h"

namespace sf {

void Type::getName(sf::StringBuf &buf)
{
	buf.append(name);
}

VoidSlice Type::instGetArray(void *inst)
{
	return { };
}

template <typename T>
struct PrimitiveImp : TypePrimitive {
	PrimitiveImp(const char *name, bool isSigned, bool isFloat)
		: TypePrimitive(name, sizeof(T), IsPod, isSigned, isFloat)
	{
	}
};

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

}
