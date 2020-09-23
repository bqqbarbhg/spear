#include "UintMap.h"

#define RHMAP_INLINE sf_inline
#include "ext/rhmap.h"

#include "sf/Reflection.h"

namespace sf {

UintMap::Iterator::Iterator(const rhmap *beginFromMap)
	: map(beginFromMap)
{
	hash = scan = 0;
	rhmap_next_inline(map, &hash, &scan, &value);
}

UintMap::Iterator &UintMap::Iterator::operator++()
{
	if (!rhmap_next_inline(map, &hash, &scan, &value)) {
		hash = scan = 0;
	}
	return *this;
}


UintMap::UintMap()
{
	rhmap_init_inline(&map);
}

UintMap::UintMap(const UintMap &rhs)
{
	rhmap_init_inline(&map);
	growImp(rhs.map.size);
	uint32_t hash = 0, scan = 0, value;
	while (rhmap_next_inline(&rhs.map, &hash, &scan, &value)) {
		rhmap_insert_inline(&map, hash, scan - 1, value);
	}
}

UintMap::UintMap(UintMap &&rhs)
	: map(rhs.map)
{
	rhmap_reset_inline(&rhs.map);
}

UintMap& UintMap::operator=(const UintMap &rhs)
{
	if (&rhs == this) return *this;
	clear();
	reserve(rhs.map.size);
	uint32_t hash = 0, scan = 0, value;
	while (rhmap_next_inline(&rhs.map, &hash, &scan, &value)) {
		rhmap_insert_inline(&map, hash, scan - 1, value);
	}
	return *this;
}

UintMap& UintMap::operator=(UintMap &&rhs)
{
	if (&rhs == this) return *this;
	memFree(map.entries);
	map = rhs.map;
	rhmap_reset_inline(&rhs.map);
	return *this;
}

UintMap::~UintMap()
{
	void *ptr = rhmap_reset_inline(&map);
	memFree(ptr);
}

void UintMap::clear()
{
	rhmap_clear_inline(&map);
}

void UintMap::growImp(uint32_t size)
{
	size_t count, allocSize;
	rhmap_grow_inline(&map, &count, &allocSize, size, 0.8);

	void *newAlloc = memAlloc(allocSize);
	void *oldAlloc = rhmap_rehash_inline(&map, count, allocSize, newAlloc);
	memFree(oldAlloc);
}

void UintMap::insertDuplicate(uint32_t key, uint32_t value)
{
	if (map.size == map.capacity) growImp(10);
	uint32_t hash = sf::hash(key);
	rhmap_insert_inline(&map, hash, 0, value);
}

bool UintMap::insertPairIfNew(uint32_t key, uint32_t value)
{
	if (map.size == map.capacity) growImp(10);
	uint32_t hash = sf::hash(key);

	uint32_t scan = 0, ref;
	while (rhmap_find_inline(&map, hash, &scan, &ref)) {
		if (ref == value) return false;
	}

	rhmap_insert_inline(&map, hash, 0, value);
	return true;
}

bool UintMap::insertOrUpdate(uint32_t key, uint32_t value)
{
	if (map.size == map.capacity) growImp(10);
	uint32_t hash = sf::hash(key);

	uint32_t scan = 0, ref;
	while (rhmap_find_inline(&map, hash, &scan, &ref)) {
		rhmap_set_inline(&map, hash, scan, value);
		return false;
	}

	rhmap_insert_inline(&map, hash, 0, value);
	return true;
}

uint32_t UintMap::findOne(uint32_t key, uint32_t missing) const
{
	uint32_t hash = sf::hash(key);
	uint32_t scan = 0;
	uint32_t value;
	if (rhmap_find_inline(&map, hash, &scan, &value)) {
		return value;
	} else {
		return missing;
	}
}

void UintMap::updateExistingOne(uint32_t key, uint32_t prevValue, uint32_t nextValue)
{
	uint32_t hash = sf::hash(key);
	rhmap_update_value_inline(&map, hash, prevValue, nextValue);
}

uint32_t UintMap::removeOne(uint32_t key, uint32_t missing)
{
	uint32_t hash = sf::hash(key);
	uint32_t scan = 0;
	uint32_t value;
	if (rhmap_find_inline(&map, hash, &scan, &value)) {
		rhmap_remove_inline(&map, hash, scan);
		return value;
	} else {
		return missing;
	}
}

void UintMap::removeExistingPair(uint32_t key, uint32_t value)
{
	uint32_t hash = sf::hash(key);
	uint32_t scan = 0;
	rhmap_find_value_inline(&map, hash, &scan, value);
	rhmap_remove_inline(&map, hash, scan);
}

bool UintMap::removePotentialPair(uint32_t key, uint32_t value)
{
	uint32_t hash = sf::hash(key);
	uint32_t scan = 0, ref;

	while (rhmap_find_inline(&map, hash, &scan, &ref)) {
		if (ref == value) {
			rhmap_remove_inline(&map, hash, scan);
			return true;
		}
	}

	return false;
}

void UintMap::removeFoundImp(uint32_t hash, uint32_t scan)
{
	sf_assert(scan > 0);
	rhmap_remove_inline(&map, hash, scan);
}

template<>
void initType<UintKeyVal>(Type *t)
{
	static Field fields[] = {
		sf_field(UintKeyVal, key),
		sf_field(UintKeyVal, val),
	};
	sf_struct(t, sf::UintKeyVal, fields, Type::CompactString | Type::IsPod);
}

struct UintMapType : Type
{
	UintMapType()
		: Type("sf::UintMap", getTypeInfo<UintMap>(), HasArray|HasArrayResize)
	{
		elementType = typeOfRecursive<UintKeyVal>();
	}

	virtual VoidSlice instGetArray(void *inst, sf::Array<char> *scratch) override
	{
		if (!scratch) return { };

		UintMap &map = *(UintMap*)inst;
		scratch->clear();
		UintKeyVal *data = (UintKeyVal*)scratch->pushUninit(sizeof(UintKeyVal) * map.map.size);

		UintKeyVal *dst = data;
		uint32_t hash = 0, scan = 0, value;
		while (rhmap_next_inline(&map.map, &hash, &scan, &value)) {
			dst->key = sf::hashReverse32(hash);
			dst->val = value;
			dst++;
		}

		return { data, map.map.size };
	}

	virtual VoidSlice instArrayReserve(void *inst, size_t size, sf::Array<char> *scratch) override
	{
		if (!scratch) return { };

		scratch->clear();
		UintKeyVal *data = (UintKeyVal*)scratch->pushUninit(sizeof(UintKeyVal) * size);
		return { data, size };
	}

	virtual void instArrayResize(void *inst, size_t size, VoidSlice elements) override
	{
		sf::Slice<UintKeyVal> data = elements.cast<UintKeyVal>();
		UintMap &map = *(UintMap*)inst;
		map.reserve((uint32_t)size);

		for (const UintKeyVal &kv : data) {
			uint32_t hash = sf::hash(kv.key);
			rhmap_insert_inline(&map.map, hash, 0, kv.val);
		}
	}
};

template<> void initType<UintMap>(Type *t) { new (t) UintMapType(); }

}
