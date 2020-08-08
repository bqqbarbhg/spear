#include "UintMap.h"

#define RHMAP_INLINE sf_inline
#include "ext/rhmap.h"

namespace sf {

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
		rhmap_insert_inline(&map, hash, scan, value);
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
	growImp(rhs.map.size);
	uint32_t hash = 0, scan = 0, value;
	while (rhmap_next_inline(&rhs.map, &hash, &scan, &value)) {
		rhmap_insert_inline(&map, hash, scan, value);
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

void UintMap::insert(uint32_t key, uint32_t value)
{
	if (map.size == map.capacity) growImp(32);
	uint32_t hash = sf::hash(key);
	rhmap_insert_inline(&map, hash, 0, value);
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

void UintMap::removePair(uint32_t key, uint32_t value)
{
	uint32_t hash = sf::hash(key);
	uint32_t scan = 0;
	rhmap_find_value_inline(&map, hash, &scan, value);
	rhmap_remove_inline(&map, hash, scan);
}

}
