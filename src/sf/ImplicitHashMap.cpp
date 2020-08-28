#include "ImplicitHashMap.h"
#include "Reflection.h"

namespace sf {

struct ImplicitHashMapType final : Type
{
	uint32_t (*hashFn)(void *inst);

	ImplicitHashMapType(const TypeInfo &info, Type *kvType, uint32_t (*hashFn)(void *inst))
		: Type("sf::ImplicitHashMap", info, HasArray | HasArrayResize)
		, hashFn(hashFn)
	{
		elementType = kvType;
	}

	virtual void getName(sf::StringBuf &buf) override
	{
		buf.append("sf::ImplicitHashMap<");
		elementType->getName(buf);
		buf.append(">");
	}

	virtual VoidSlice instGetArray(void *inst, sf::Array<char> *scratch) override
	{
		ImplicitHashMapBase *map = (ImplicitHashMapBase*)inst;
		return { map->data, map->map.size };
	}

	virtual VoidSlice instArrayReserve(void *inst, size_t size, sf::Array<char> *scratch) override
	{
		uint32_t kvSize = elementType->info.size;
		Type *elemType = elementType;

		ImplicitHashMapBase *map = (ImplicitHashMapBase*)inst;
		if (map->map.capacity < size) {
			size_t count, allocSize;
			rhmap_grow(&map->map, &count, &allocSize, size, 0.8);

			void *newAlloc = memAlloc(allocSize + count * kvSize);
			void *newData = (char*)newAlloc + allocSize;
			if (map->map.size) {
				elemType->info.moveRange(newData, map->data, map->map.size);
			}
			map->data = newData;

			void *oldAlloc = rhmap_rehash(&map->map, count, allocSize, newAlloc);
			memFree(oldAlloc);
		}

		if (size > map->map.size) {
			char *base = (char*)map->data + map->map.size * kvSize;
			elemType->info.constructRange(base, size - map->map.size);
		}

		return { map->data, map->map.capacity };
	}

	virtual void instArrayResize(void *inst, size_t size, VoidSlice elements) override
	{
		uint32_t kvSize = elementType->info.size;

		ImplicitHashMapBase *map = (ImplicitHashMapBase*)inst;
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

void initImplicitHashMapType(Type *t, const TypeInfo &info, Type *kvType, uint32_t (*hashFn)(void *inst))
{
	new (t) ImplicitHashMapType(info, kvType, hashFn);
}

}
