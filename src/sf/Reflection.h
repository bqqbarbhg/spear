#pragma once

#include "Base.h"
#include "String.h"
#include "Array.h"

namespace sf {

struct Type;
struct TypeArray;

struct Field {
	sf::CString name;
	uint32_t offset;
	uint32_t size;
	Type *type;
};

struct Type {

	enum Primitive {
		Bool, Char,
		I8, I16, I32, I64,
		U8, U16, U32, U64,
		F32, F64,
	};

	enum Flag {
		HasArray = 0x1,
		HasFields = 0x2,
		IsPod = 0x4,
		HasArrayResize = 0x8,
		HasString = 0x10,
		IsPrimitive = 0x20,
		CompactString = 0x40,
	};

	sf::CString name;
	uint32_t size;
	uint32_t flags;
	Primitive primitive = Bool;

	Slice<const Field> fields;
	Type *elementType = nullptr;

	Type(const char *name, size_t size, uint32_t flags)
		: name(name), size((uint32_t)size), flags(flags)
	{
	}

	virtual void getName(sf::StringBuf &buf);

	virtual void instConstruct(void *inst, size_t num);
	virtual void instMove(void *dst, void *src, size_t num);
	virtual void instDestruct(void *inst, size_t num);

	virtual VoidSlice instGetArray(void *inst);
	virtual VoidSlice instArrayReserve(void *inst, size_t size);
	virtual void instArrayResize(void *inst, size_t size);
};

template <typename T>
struct TypeStruct : Type {

	TypeStruct(const char *name, sf::Slice<const Field> fields, uint32_t userFlags=0)
		: Type(name, sizeof(T), HasFields|userFlags)
	{
		if (IsCopyable<T>::value) flags |= IsPod;
		this->fields = fields;
	}

	virtual void instConstruct(void *inst, size_t num)
	{
		constructRangeImp<T>(inst, num);
	}

	virtual void instMove(void *dst, void *src, size_t num)
	{
		moveRangeImp<T>(dst, src, num);
	}

	virtual void instDestruct(void *inst, size_t num)
	{
		destructRangeImp<T>(inst, num);
	}
};

#define sf_field(type, name) Field{ sf::CString(::sf::Const, #name, sizeof(#name) - 1), offsetof(type, name), sizeof(type::name), ::sf::typeOfRecursive<decltype(type::name)>() }
#define sf_struct(t, type, ...) new (t) ::sf::TypeStruct<type>(#type, __VA_ARGS__)

void writeInstBinary(sf::Array<char> &dst, void *inst, Type *type);
bool readInstBinary(sf::Slice<char> &src, void *inst, Type *type);

template <typename T> sf_inline void writeBinary(sf::Array<char> &dst, T &t) { writeInstBinary(dst, &t, typeOf<T>()); }
template <typename T> sf_inline bool readBinary(sf::Slice<char> &src, T &t) { return readInstBinary(src, &t, typeOf<T>()); }

}
