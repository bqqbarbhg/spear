#pragma once

#include "Base.h"
#include "ext/rhmap.h"

namespace sf {

struct UintFind
{
	const rhmap *map;
	uint32_t hash;
	uint32_t scan;

	sf_forceinline bool next(uint32_t &result) {
		return rhmap_find(map, hash, &scan, &result);
	}
};

struct UintMap
{
	rhmap map;

	UintMap();
	UintMap(const UintMap &rhs);
	UintMap(UintMap &&rhs);
	UintMap& operator=(const UintMap &rhs);
	UintMap& operator=(UintMap &&rhs);
	~UintMap();

	sf_forceinline uint32_t size() const { return map.size; }
	sf_forceinline uint32_t capacity() const { return map.capacity; }

	void insert(uint32_t key, uint32_t value);
	uint32_t findOne(uint32_t key, uint32_t missing) const;

	void removePair(uint32_t key, uint32_t value);

	sf_forceinline UintFind findAll(uint32_t key) const {
		UintFind find;
		find.map = &map;
		find.hash = sf::hash(key);
		find.scan = 0;
		return find;
	}

	void clear();

	void reserve(uint32_t size)
	{
		if (size > map.capacity) {
			growImp(size);
		}
	}

	void growImp(uint32_t size);
};

}
