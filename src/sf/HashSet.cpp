#include "HashSet.h"
#include "Reflection.h"

namespace sf {

struct HashSetType final : Type
{
	uint32_t (*hashFn)(void *inst);

	HashSetType(const TypeInfo &info, Type *kvType, uint32_t (*hashFn)(void *inst))
		: Type("sf::HashSet", info, HasArray | HasArrayResize)
		, hashFn(hashFn)
	{
		elementType = kvType;
	}

	virtual void getName(sf::StringBuf &buf) override
	{
		buf.append("sf::HashSet<");
		elementType->getName(buf);
		buf.append(">");
	}

	virtual VoidSlice instGetArray(void *inst, sf::Array<char> *scratch) override
	{
		HashSetBase *set = (HashSetBase*)inst;
		return { set->data, set->map.size };
	}

	virtual VoidSlice instArrayReserve(void *inst, size_t size, sf::Array<char> *scratch) override
	{
		uint32_t entrySize = elementType->info.size;

		HashSetBase *set = (HashSetBase*)inst;
		if (set->map.capacity < size) {
			size_t count, allocSize;
			rhmap_grow(&set->map, &count, &allocSize, size, 0.8);

			void *newAlloc = memAlloc(allocSize + count * entrySize);
			void *newData = (char*)newAlloc + allocSize;
			if (set->map.size) {
				elementType->info.moveRange(newData, set->data, set->map.size);
			}
			set->data = newData;

			void *oldAlloc = rhmap_rehash(&set->map, count, allocSize, newAlloc);
			memFree(oldAlloc);
		}

		if (size > set->map.size) {
			char *base = (char*)set->data + set->map.size * entrySize;
			elementType->info.constructRange(base, size - set->map.size);
		}

		return { set->data, set->map.capacity };
	}

	virtual void instArrayResize(void *inst, size_t size, VoidSlice elements) override
	{
		uint32_t entrySize = elementType->info.size;

		HashSetBase *set = (HashSetBase*)inst;
		sf_assert(set->map.capacity >= size);
		if (set->map.size > size) {
			char *base = (char*)set->data + set->map.size * entrySize;
			char *ptr = base;
			for (uint32_t i = set->map.size; i < size; i++) {
				uint32_t h = hashFn(ptr), scan = 0;
				rhmap_find_value(&set->map, h, &scan, i);
				rhmap_remove(&set->map, h, scan);
				ptr += entrySize;
			}
			elementType->info.destructRange(base, set->map.size - size);
		}
		char *ptr = (char*)set->data + set->map.size * entrySize;
		for (uint32_t i = set->map.size; i < size; i++) {
			uint32_t h = hashFn(ptr), scan = 0;
			rhmap_insert(&set->map, h, scan, i);
			ptr += entrySize;
		}
	}
};

void initHashSetType(Type *t, const TypeInfo &info, Type *kvType, uint32_t (*hashFn)(void *inst))
{
	new (t) HashSetType(info, kvType, hashFn);
}

}
