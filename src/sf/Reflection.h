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
	sf::CString name;
	uint32_t size;
	uint32_t flags;

	Slice<const Field> fields;
	Type *elementType = nullptr;

	enum Flag {
		HasArray = 0x1,
		HasFields = 0x2,
		IsPod = 0x4,
	};

	Type(const char *name, size_t size, uint32_t flags)
		: name(name), size(size), flags(flags)
	{
	}

	virtual void getName(sf::StringBuf &buf);

	virtual VoidSlice instGetArray(void *inst);
};

struct TypePrimitive : Type {
	bool isSigned;
	bool isFloat;

	TypePrimitive(const char *name, size_t size, uint32_t flags, bool isSigned, bool isFloat)
		: Type(name, size, flags), isSigned(isSigned), isFloat(isFloat)
	{
	}
};

struct TypeStruct : Type {
	sf::Slice<const Field> fields;

	TypeStruct(const char *name, size_t size, sf::Slice<const Field> fields, uint32_t flags)
		: Type(name, size, HasFields | flags), fields(fields)
	{
	}
};

#define sf_field(type, name) Field{ sf::CString(::sf::Const, #name, sizeof(#name) - 1), offsetof(type, name), sizeof(type::name), ::sf::typeOf<decltype(type::name)>() }
#define sf_struct(type, fields) new TypeStruct(#type, sizeof(type), fields, ::sf::IsCopyable<type>::value ? ::sf::Type::IsPod : 0)

}
