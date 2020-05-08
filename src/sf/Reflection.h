#pragma once

#include "Base.h"
#include "String.h"
#include "Array.h"

namespace sf {

struct Type;
struct TypeArray;

struct Field {
	enum Flag {
		CompactString = 0x01,
	};

	sf::CString name;
	uint32_t offset;
	uint32_t size;
	Type *type;
	uint32_t flags;
};

struct PolymorphType
{
	sf::CString name;
	uint32_t value;
	Type *type;
};

struct PolymorphInstance
{
	void *inst = nullptr;
	const PolymorphType *type = nullptr;
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
		HasSetString = 0x80,
		PolymorphBase = 0x100,
		Polymorph = 0x200,
		Initialized = 0x400,
	};

	Type *next = nullptr;
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

	virtual void init();

	virtual void getName(sf::StringBuf &buf);
	virtual sf::CString getPolymorphTagName();

	virtual void instConstruct(void *inst, size_t num);
	virtual void instMove(void *dst, void *src, size_t num);
	virtual void instDestruct(void *inst, size_t num);

	virtual VoidSlice instGetArray(void *inst);
	virtual VoidSlice instArrayReserve(void *inst, size_t size);
	virtual void instArrayResize(void *inst, size_t size);

	virtual void instSetString(void *inst, sf::String str);

	virtual const PolymorphType *getPolymorphTypeByValue(uint32_t value);
	virtual const PolymorphType *getPolymorphTypeByName(sf::String name);

	virtual PolymorphInstance instGetPolymorph(void *inst);
	virtual void *instSetPolymorph(void *inst, Type *type);
};

template <typename T>
struct TypeStruct final : Type {

	TypeStruct(const char *name, sf::Slice<const Field> fields, uint32_t userFlags=0)
		: Type(name, sizeof(T), HasFields|userFlags)
	{
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

struct TypePolymorphicStructBase : Type {
	TypePolymorphicStructBase(const char *name, size_t size, ::sf::Slice<const PolymorphType> types, size_t tagOffset, const char *tagName, uint32_t userFlags);

	virtual sf::CString TypePolymorphicStructBase::getPolymorphTagName();

	virtual const PolymorphType *getPolymorphTypeByValue(uint32_t value);
	virtual const PolymorphType *getPolymorphTypeByName(sf::String name);

	virtual PolymorphInstance instGetPolymorph(void *inst);

	struct Data;
	Data *data;
};

template <typename T>
struct TypePolymorphicStruct final : TypePolymorphicStructBase {

	TypePolymorphicStruct(const char *name, size_t tagOffset, const char *tagName, sf::Slice<const Field> fields, ::sf::Slice<const PolymorphType> types, uint32_t userFlags=0)
		: TypePolymorphicStructBase(name, sizeof(T), types, tagOffset, tagName, userFlags)
	{
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

struct EnumValue
{
	sf::CString name;
	uint32_t value;
};

struct TypeEnum final : Type {

	struct Data;
	Data *data;

	TypeEnum(const char *name, size_t size, sf::Slice<const EnumValue> values, uint32_t userFlags=0);

	virtual VoidSlice instGetArray(void *inst);
	virtual void instSetString(void *inst, sf::String str);

};

#define sf_enum(type, name) ::sf::EnumValue{ #name, (uint32_t)type::name }
#define sf_enum_type(t, type, ...) new (t) ::sf::TypeEnum(#type, sizeof(type), __VA_ARGS__)

#define sf_poly(type, name, value) ::sf::PolymorphType{ #name, (uint32_t)type::name, ::sf::typeOfRecursive<value>() }

#define sf_field(type, name) Field{ sf::CString(::sf::Const, #name, sizeof(#name) - 1), offsetof(type, name), sizeof(type::name), ::sf::typeOfRecursive<decltype(type::name)>(), 0 }
#define sf_field_flags(type, name, flags) Field{ sf::CString(::sf::Const, #name, sizeof(#name) - 1), offsetof(type, name), sizeof(type::name), ::sf::typeOfRecursive<decltype(type::name)>(), (flags) }
#define sf_struct(t, type, ...) new (t) ::sf::TypeStruct<type>(#type, __VA_ARGS__)
#define sf_struct_poly(t, type, tag, ...) new (t) ::sf::TypePolymorphicStruct<type>(#type, offsetof(type, tag), #tag, __VA_ARGS__)

void writeInstBinary(sf::Array<char> &dst, void *inst, Type *type);
bool readInstBinary(sf::Slice<char> &src, void *inst, Type *type);

template <typename T> sf_inline void writeBinary(sf::Array<char> &dst, T &t) { writeInstBinary(dst, &t, typeOf<T>()); }
template <typename T> sf_inline bool readBinary(sf::Slice<char> &src, T &t) { return readInstBinary(src, &t, typeOf<T>()); }

}
