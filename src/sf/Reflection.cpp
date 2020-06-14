#include "Reflection.h"
#include "Vector.h"
#include "ext/mx/mx_platform.h"
#include "sf/HashMap.h"

namespace sf {

static_assert(sizeof(Type) + 2*sizeof(void*) <= MaxTypeStructSize, "Type size plus user data is too large");

void Type::init()
{
}

void Type::getName(sf::StringBuf &buf)
{
	buf.append(name);
}

sf::CString Type::getPolymorphTagName()
{
	return elementType->getPolymorphTagName();
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

void Type::instSetString(void *inst, sf::String str)
{
	sf_assert(0 && "Set string not supported");
}

const PolymorphType *Type::getPolymorphTypeByValue(uint32_t value)
{
	return nullptr;
}

const PolymorphType *Type::getPolymorphTypeByName(sf::String name)
{
	return nullptr;
}

void *Type::instGetPointer(void *inst)
{
	return *(void**)inst;
}

void *Type::instSetPointer(void *inst)
{
	sf_assert(0 && "Set pointer not supported");
	return nullptr;
}

PolymorphInstance Type::instGetPolymorph(void *inst)
{
	return { };
}

void *Type::instSetPolymorph(void *inst, Type *type)
{
	sf_assert(0 && "Set polymorph not supported");
	return nullptr;
}

struct TypeEnum::Data
{
	sf::HashMap<sf::CString, uint32_t> stringToValue;
	sf::HashMap<uint32_t, sf::CString> valueToString;
};

TypeEnum::TypeEnum(const char *name, const TypeInfo &info, sf::Slice<const EnumValue> values, uint32_t userFlags)
	: Type(name, info, IsPod|HasString|HasSetString|userFlags), data(new Data())
{
	sf_assert(info.size == sizeof(uint32_t));
	data->stringToValue.reserve((uint32_t)values.size);
	data->valueToString.reserve((uint32_t)values.size);
	for (const EnumValue &val : values) {
		data->stringToValue.insert(val.name, val.value);
		data->valueToString.insert(val.value, val.name);
	}
}

VoidSlice TypeEnum::instGetArray(void *inst)
{
	uint32_t value = *(uint32_t*)inst;
	auto it = data->valueToString.find(value);
	sf_assert(it);
	return it->val.slice();
}

void TypeEnum::instSetString(void *inst, sf::String str)
{
	auto it = data->stringToValue.find(str);
	sf_assert(it);
	*(uint32_t*)inst = it->val;
}

struct TypePolymorphicStructBase::Data
{
	sf::HashMap<uint32_t, sf::CString> valueToName;
	sf::HashMap<sf::CString, const PolymorphType*> nameToType;
	sf::HashMap<uint32_t, const PolymorphType*> valueToType;
	size_t tagOffset;
	sf::CString tagName;

	Data(Slice<const PolymorphType> types, size_t tagOffset, const char *tagName)
		: tagOffset(tagOffset), tagName(tagName)
	{
		nameToType.reserve((uint32_t)types.size);
		valueToType.reserve((uint32_t)types.size);
		valueToName.reserve((uint32_t)types.size);
		for (const PolymorphType &t : types) {
			valueToName.insert(t.value, t.name);
			nameToType.insert(t.name, &t);
			valueToType.insert(t.value, &t);
		}
	}
};

TypePolymorphicStructBase::TypePolymorphicStructBase(const char *name, const TypeInfo &info, Slice<const PolymorphType> types, size_t tagOffset, const char *tagName, uint32_t userFlags)
	: Type(name, info, PolymorphBase|HasFields|userFlags), data(new Data(types, tagOffset, tagName))
{
}

sf::CString TypePolymorphicStructBase::getPolymorphTagName()
{
	return data->tagName;
}

const PolymorphType *TypePolymorphicStructBase::getPolymorphTypeByValue(uint32_t value)
{
	auto it = data->valueToType.find(value);
	return it ? it->val : nullptr;
}

const PolymorphType *TypePolymorphicStructBase::getPolymorphTypeByName(sf::String name)
{
	auto it = data->nameToType.find(name);
	return it ? it->val : nullptr;
}

PolymorphInstance TypePolymorphicStructBase::instGetPolymorph(void *inst)
{
	uint32_t tag = *(uint32_t*)((char*)inst + data->tagOffset);
	auto it = data->valueToType.find(tag);
	return { inst, it ? it->val : nullptr };
}

static constexpr const uint32_t PrimitiveFlags = Type::IsPrimitive|Type::IsPod|Type::CompactString;
template<> void initType<bool>(Type *t) { new (t) Type("bool", getTypeInfo<bool>(), PrimitiveFlags); t->primitive = Type::Bool; }
template<> void initType<char>(Type *t) { new (t) Type("char", getTypeInfo<char>(), PrimitiveFlags); t->primitive = Type::Char; }
template<> void initType<int8_t>(Type *t) { new (t) Type("int8_t", getTypeInfo<int8_t>(), PrimitiveFlags); t->primitive = Type::I8; }
template<> void initType<int16_t>(Type *t) { new (t) Type("int16_t", getTypeInfo<int16_t>(), PrimitiveFlags); t->primitive = Type::I16; }
template<> void initType<int32_t>(Type *t) { new (t) Type("int32_t", getTypeInfo<int32_t>(), PrimitiveFlags); t->primitive = Type::I32; }
template<> void initType<int64_t>(Type *t) { new (t) Type("int64_t", getTypeInfo<int64_t>(), PrimitiveFlags); t->primitive = Type::I64; }
template<> void initType<uint8_t>(Type *t) { new (t) Type("uint8_t", getTypeInfo<uint8_t>(), PrimitiveFlags); t->primitive = Type::U8; }
template<> void initType<uint16_t>(Type *t) { new (t) Type("uint16_t", getTypeInfo<uint16_t>(), PrimitiveFlags); t->primitive = Type::U16; }
template<> void initType<uint32_t>(Type *t) { new (t) Type("uint32_t", getTypeInfo<uint32_t>(), PrimitiveFlags); t->primitive = Type::U32; }
template<> void initType<uint64_t>(Type *t) { new (t) Type("uint64_t", getTypeInfo<uint64_t>(), PrimitiveFlags); t->primitive = Type::U64; }
template<> void initType<float>(Type *t) { new (t) Type("float", getTypeInfo<float>(), PrimitiveFlags); t->primitive = Type::F32; }
template<> void initType<double>(Type *t) { new (t) Type("double", getTypeInfo<double>(), PrimitiveFlags); t->primitive = Type::F64; }

void writeInstBinary(sf::Array<char> &dst, void *inst, Type *type)
{
	uint32_t flags = type->flags;
	char *base = (char*)inst;
	if (flags & Type::IsPod) {
		dst.push(base, type->info.size);
	} else if (flags & Type::Polymorph) {

		sf::PolymorphInstance poly = type->instGetPolymorph(inst);
		if (poly.type) {
			uint32_t tagValue = (uint32_t)poly.type->value;
			dst.push((char*)&tagValue, sizeof(uint32_t));
			writeInstBinary(dst, poly.inst, poly.type->type);
		} else {
			uint32_t tagValue = ~0u;
			dst.push((char*)&tagValue, sizeof(uint32_t));
		}

	} else if (flags & Type::HasFields) {
		for (const Field &field : type->fields) {
			writeInstBinary(dst, base + field.offset, field.type);
		}
	} else if (flags & Type::HasArray) {
		Type *elem = type->elementType;
		size_t elemSize = elem->info.size;
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
		if (src.size < type->info.size) return false;
		memcpy(inst, src.data, type->info.size);
		src = src.drop(type->info.size);
	} else if (flags & Type::Polymorph) {

		uint32_t tagValue;
		if (src.size < sizeof(uint32_t)) return false;
		memcpy(&tagValue, src.data, sizeof(uint32_t));
		src = src.drop(sizeof(uint32_t));

		const PolymorphType *polyType = type->elementType->getPolymorphTypeByValue(tagValue);
		void *ptr = type->instSetPolymorph(inst, polyType->type);
		return readInstBinary(src, ptr, polyType->type);

	} else if (flags & Type::HasFields) {
		for (const Field &field : type->fields) {
			if (!readInstBinary(src, base + field.offset, field.type)) return false;
		}
	} else if (flags & Type::HasArrayResize) {
		Type *elem = type->elementType;
		size_t elemSize = elem->info.size;
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
	} else if (flags & Type::HasSetString) {
		uint32_t size;

		if (src.size < sizeof(uint32_t)) return false;
		memcpy(&size, src.data, sizeof(uint32_t));
		src = src.drop(sizeof(uint32_t));

		if (src.size < size) return false;
		type->instSetString(inst, src.take(size));
		src = src.drop(size);
	} else {
		// TODO: Serialization callback
		sf_assert(0 && "Cannot serialize type");
	}

	return true;
}

uint32_t hashInstReflected(void *inst, Type *type)
{
	uint32_t flags = type->flags;
	char *base = (char*)inst;
	if (flags & Type::IsPod) {
		return sf::hashBuffer(base, type->info.size);
	} else if (flags & Type::Polymorph) {

		sf::PolymorphInstance poly = type->instGetPolymorph(inst);
		if (poly.type) {
			uint32_t h = hash(poly.type->value);
			return hashCombine(h, hashInstReflected(poly.inst, poly.type->type));
		} else {
			return 0;
		}

	} else if (flags & Type::HasFields) {
		uint32_t h = 0;
		for (const Field &field : type->fields) {
			h = hashCombine(h, hashInstReflected(base + field.offset, field.type));
		}
		return h;
	} else if (flags & Type::HasArray) {
		Type *elem = type->elementType;
		size_t elemSize = elem->info.size;
		VoidSlice slice = type->instGetArray(inst);
		uint32_t size = (uint32_t)slice.size;
		uint32_t h = sf::hash(size);
		if (elem->flags & Type::IsPod) {
			h = sf::hashCombine(h, sf::hashBuffer(slice.data, slice.size * elemSize));
		} else {
			char *ptr = (char*)slice.data;
			for (uint32_t i = 0; i < size; i++) {
				h = sf::hashCombine(h, hashInstReflected(ptr, elem));
				ptr += elemSize;
			}
		}
		return h;
	} else {
		// TODO: Hash callback?
		sf_assert(0 && "Cannot hash type");
		return 0;
	}
}

int compareInstReflected(void *a, void *b, Type *type)
{
	uint32_t flags = type->flags;
	char *ba = (char*)a, *bb = (char*)b;
	if (flags & Type::IsPod) {
		return memcmp(ba, bb, type->info.size);
	} else if (flags & Type::Polymorph) {

		sf::PolymorphInstance pa = type->instGetPolymorph(a);
		sf::PolymorphInstance pb = type->instGetPolymorph(b);
		if (!pa.type || !pb.type) {
			if (!pa.type && !pb.type) return 0;
			return pa.type ? +1 : -1;
		}
		if (pa.type->value != pb.type->value) {
			return pa.type->value < pb.type->value ? -1 : +1;
		}

		return compareInstReflected(pa.inst, pb.inst, pa.type->type);

	} else if (flags & Type::HasFields) {
		uint32_t h = 0;
		for (const Field &field : type->fields) {
			int cmp = compareInstReflected(ba + field.offset, bb + field.offset, field.type);
			if (cmp != 0) return cmp;
		}
		return 0;
	} else if (flags & Type::HasArray) {
		Type *elem = type->elementType;
		size_t elemSize = elem->info.size;
		VoidSlice sa = type->instGetArray(a);
		VoidSlice sb = type->instGetArray(b);
		uint32_t size = (uint32_t)sa.size;
		if (sb.size != size) {
			return size < sb.size ? -1 : +1;
		}
		if (elem->flags & Type::IsPod) {
			return memcmp(sa.data, sb.data, size * elemSize);
		} else {
			char *pa = (char*)sa.data, *pb = (char*)sb.data;
			for (uint32_t i = 0; i < size; i++) {
				int cmp = compareInstReflected(pa, pb, elem);
				if (cmp != 0) return cmp;
				pa += elemSize;
				pb += elemSize;
			}
		}
		return 0;
	} else {
		// TODO: Hash callback?
		sf_assert(0 && "Cannot compare type");
		return 0;
	}
}

struct ReflectedType final : Type
{
	struct Data
	{
		Field fields[1];
	};
	Data *data;

	ReflectedType(const TypeInfo &info, Data *data)
		: Type("sf::Reflected", info, HasFields)
		, data(data)
	{
		fields = data->fields;
	}

	virtual void getName(sf::StringBuf &buf)
	{
		buf.append("sf::Reflected<");
		data->fields[0].type->getName(buf);
		buf.append(">");
	}
};

void initReflectedType(Type *t, const TypeInfo &info, Type *elemType)
{
	ReflectedType::Data *data = new ReflectedType::Data();
	data->fields[0].name = "data";
	data->fields[0].offset = 0;
	data->fields[0].size = (uint32_t)elemType->info.size;
	data->fields[0].type = elemType;
	new (t) ReflectedType(info, data);
	t->flags |= elemType->flags & (Type::IsPod|Type::CompactString);
}

}
