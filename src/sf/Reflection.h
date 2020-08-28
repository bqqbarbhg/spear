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

typedef void (*PostSerializeFn)(void *inst, sf::Type *type);

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
		HasPointer = 0x800,
		IsBox = 0x1000,
	};

	Type *next = nullptr;
	Type *baseType = nullptr;
	sf::CString name;
	TypeInfo info;
	uint32_t flags;
	Primitive primitive = Bool;

	PostSerializeFn postSerializeFn = nullptr;

	Slice<const Field> fields;
	Type *elementType = nullptr;

	Type(const char *name, const TypeInfo &info, uint32_t flags)
		: name(name), info(info), flags(flags)
	{
	}

	virtual void init();

	virtual void getName(sf::StringBuf &buf);
	virtual sf::CString getPolymorphTagName();

	virtual void getPolymorphTypeNames(sf::Array<sf::StringBuf> &names);

	virtual VoidSlice instGetArray(void *inst, sf::Array<char> *scartch=nullptr);
	virtual VoidSlice instArrayReserve(void *inst, size_t size, sf::Array<char> *scartch=nullptr);
	virtual void instArrayResize(void *inst, size_t size, VoidSlice elements);

	virtual void instSetString(void *inst, sf::String str);

	virtual const PolymorphType *getPolymorphTypeByValue(uint32_t value);
	virtual const PolymorphType *getPolymorphTypeByName(sf::String name);

	virtual void *instGetPointer(void *inst);
	virtual void *instSetPointer(void *inst);

	virtual PolymorphInstance instGetPolymorph(void *inst);
	virtual void *instSetPolymorph(void *inst, Type *type);
};

struct TypeStruct final : Type {

	TypeStruct(const char *name, const TypeInfo &info, Type *baseType, sf::Slice<const Field> fields, uint32_t userFlags=0)
		: Type(name, info, HasFields|userFlags)
	{
		this->fields = fields;
		this->baseType = baseType;
	}
};

struct TypePolymorphicStructBase : Type {
	TypePolymorphicStructBase(const char *name, const TypeInfo &info, ::sf::Slice<const PolymorphType> types, size_t tagOffset, const char *tagName, uint32_t userFlags);

	virtual sf::CString getPolymorphTagName() override;

	virtual void getPolymorphTypeNames(sf::Array<sf::StringBuf> &names) override;

	virtual const PolymorphType *getPolymorphTypeByValue(uint32_t value) override;
	virtual const PolymorphType *getPolymorphTypeByName(sf::String name) override;

	virtual PolymorphInstance instGetPolymorph(void *inst) override;

	struct Data;
	Data *data;
};

struct TypePolymorphicStruct final : TypePolymorphicStructBase {

	TypePolymorphicStruct(const char *name, const TypeInfo &info, size_t tagOffset, const char *tagName, sf::Slice<const Field> fields, ::sf::Slice<const PolymorphType> types, uint32_t userFlags=0)
		: TypePolymorphicStructBase(name, info, types, tagOffset, tagName, userFlags)
	{
		this->fields = fields;
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

	TypeEnum(const char *name, const TypeInfo &info, sf::Slice<const EnumValue> values, uint32_t userFlags=0);

	virtual VoidSlice instGetArray(void *inst, sf::Array<char> *scratch) override;
	virtual void instSetString(void *inst, sf::String str) override;

};

#define sf_enum(type, name) ::sf::EnumValue{ #name, (uint32_t)type::name }
#define sf_enum_type(t, type, ...) new (t) ::sf::TypeEnum(#type, ::sf::getTypeInfo<type>(), __VA_ARGS__)

#define sf_poly(type, name, value) ::sf::PolymorphType{ #name, (uint32_t)type::name, ::sf::typeOfRecursive<value>() }

#define sf_field(type, name) Field{ sf::CString(::sf::Const, #name, sizeof(#name) - 1), offsetof(type, name), sizeof(type::name), ::sf::typeOfRecursive<decltype(type::name)>(), 0 }
#define sf_field_flags(type, name, flags) Field{ sf::CString(::sf::Const, #name, sizeof(#name) - 1), offsetof(type, name), sizeof(type::name), ::sf::typeOfRecursive<decltype(type::name)>(), (flags) }
#define sf_struct(t, type, ...) new (t) ::sf::TypeStruct(#type, ::sf::getTypeInfo<type>(), nullptr, __VA_ARGS__)
#define sf_struct_base(t, type, base, ...) new (t) ::sf::TypeStruct(#type, ::sf::getTypeInfo<type>(), ::sf::typeOfRecursive<base>(), __VA_ARGS__)
#define sf_struct_poly(t, type, tag, ...) new (t) ::sf::TypePolymorphicStruct(#type, ::sf::getTypeInfo<type>(), offsetof(type, tag), #tag, __VA_ARGS__)

void writeInstBinary(sf::Array<char> &dst, void *inst, Type *type);
bool readInstBinary(sf::Slice<const char> &src, void *inst, Type *type);
uint32_t hashInstReflected(void *inst, Type *type);
int compareInstReflected(void *a, void *b, Type *type);

template <typename T> sf_inline void writeBinary(sf::Array<char> &dst, T &t) { writeInstBinary(dst, (void*)&t, typeOf<T>()); }
template <typename T> sf_inline bool readBinary(sf::Slice<const char> &src, T &t) { return readInstBinary(src, &t, typeOf<T>()); }
template <typename T> sf_inline uint32_t hashReflected(T &t) { return hashInstReflected((void*)&t, typeOf<T>()); }
template <typename T> sf_inline int compareReflected(T &a, T &b) { return compareInstReflected((void*)&a, (void*)&b, typeOf<T>()); }

template <typename T>
struct Reflected
{
	T data;

	sf_forceinline bool operator==(const Reflected &rhs) const { return compareReflected(data, rhs.data) == 0; }
	sf_forceinline bool operator!=(const Reflected &rhs) const { return compareReflected(data, rhs.data) != 0; }
	sf_forceinline bool operator<(const Reflected &rhs) const { return compareReflected(data, rhs.data) < 0; }
	sf_forceinline bool operator<=(const Reflected &rhs) const { return compareReflected(data, rhs.data) <= 0; }
	sf_forceinline bool operator>(const Reflected &rhs) const { return compareReflected(data, rhs.data) > 0; }
	sf_forceinline bool operator>=(const Reflected &rhs) const { return compareReflected(data, rhs.data) >= 0; }
};

template <typename T>
sf_inline uint32_t hash(const Reflected<T> &t) { return hashReflected(t.data); }

void initReflectedType(Type *t, const TypeInfo &info, Type *elemType);

template <typename T>
struct InitType<Reflected<T>> {
	static void init(Type *t) { return initReflectedType(t, getTypeInfo<Reflected<T>>(), typeOfRecursive<T>()); }
};

}
