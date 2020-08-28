#include "HashMap.h"
#include "Reflection.h"

namespace sf {

struct KeyValType final : Type
{
	struct Data
	{
		Field fields[2];
	};
	Data *data;

	KeyValType(const TypeInfo &info, Data *data)
		: Type("sf::KeyVal", info, HasFields)
		, data(data)
	{
		fields = data->fields;
	}

	virtual void getName(sf::StringBuf &buf) override
	{
		buf.append("sf::KeyVal<");
		data->fields[0].type->getName(buf);
		buf.append(", ");
		data->fields[1].type->getName(buf);
		buf.append(">");
	}
};

void initKeyValType(Type *t, const TypeInfo &info ,Type *keyType, Type *valType, size_t valOffset)
{
	KeyValType::Data *data = new KeyValType::Data();
	data->fields[0].name = "key";
	data->fields[0].offset = 0;
	data->fields[0].size = (uint32_t)keyType->info.size;
	data->fields[0].type = keyType;
	data->fields[1].name = "val";
	data->fields[1].offset = (uint32_t)valOffset;
	data->fields[1].size = (uint32_t)valType->info.size;
	data->fields[1].type = valType;
	new (t) KeyValType(info, data);
	t->flags |= keyType->flags & valType->flags & (Type::IsPod|Type::CompactString);
}

struct HashMapType final : Type
{
	uint32_t (*hashFn)(void *inst);

	HashMapType(const TypeInfo &info, Type *kvType, uint32_t (*hashFn)(void *inst))
		: Type("sf::HashMap", info, HasArray | HasArrayResize)
		, hashFn(hashFn)
	{
		elementType = kvType;
	}

	virtual void getName(sf::StringBuf &buf) override
	{
		buf.append("sf::HashMap<");
		elementType->fields[0].type->getName(buf);
		buf.append(", ");
		elementType->fields[1].type->getName(buf);
		buf.append(">");
	}

	virtual VoidSlice instGetArray(void *inst, sf::Array<char> *scratch) override
	{
		HashMapBase *map = (HashMapBase*)inst;
		return { map->data, map->map.size };
	}

	virtual VoidSlice instArrayReserve(void *inst, size_t size, sf::Array<char> *scratch) override
	{
		uint32_t kvSize = elementType->info.size;
		KeyValType *kvType = (KeyValType*)elementType;

		HashMapBase *map = (HashMapBase*)inst;
		if (map->map.capacity < size) {
			size_t count, allocSize;
			rhmap_grow(&map->map, &count, &allocSize, size, 0.8);

			void *newAlloc = memAlloc(allocSize + count * kvSize);
			void *newData = (char*)newAlloc + allocSize;
			if (map->map.size) {
				kvType->info.moveRange(newData, map->data, map->map.size);
			}
			map->data = newData;

			void *oldAlloc = rhmap_rehash(&map->map, count, allocSize, newAlloc);
			memFree(oldAlloc);
		}

		if (size > map->map.size) {
			char *base = (char*)map->data + map->map.size * kvSize;
			kvType->info.constructRange(base, size - map->map.size);
		}

		return { map->data, map->map.capacity };
	}

	virtual void instArrayResize(void *inst, size_t size, VoidSlice elements) override
	{
		uint32_t kvSize = elementType->info.size;

		HashMapBase *map = (HashMapBase*)inst;
		sf_assert(map->map.capacity >= size);
		if (map->map.size > size) {
			char *base = (char*)map->data + map->map.size * kvSize;
			char *ptr = base;
			for (uint32_t i = map->map.size; i < size; i++) {
				uint32_t h = hashFn(ptr), scan = 0;
				rhmap_find_value(&map->map, h, &scan, i);
				rhmap_remove(&map->map, h, scan);
				ptr += kvSize;
			}
			elementType->info.destructRange(base, map->map.size - size);
		}
		char *ptr = (char*)map->data + map->map.size * kvSize;
		for (uint32_t i = map->map.size; i < size; i++) {
			uint32_t h = hashFn(ptr), scan = 0;
			rhmap_insert(&map->map, h, scan, i);
			ptr += kvSize;
		}
	}
};

void initHashMapType(Type *t, const TypeInfo &info, Type *kvType, uint32_t (*hashFn)(void *inst))
{
	new (t) HashMapType(info, kvType, hashFn);
}

}
