#pragma once

#include "Base.h"
#include "ext/rhset.h"

namespace sf {

struct UintSet
{
	struct Iterator
	{
		const rhset *set;
		uint32_t hash;
		uint32_t scan;

		Iterator(const rhset *beginFromSet);
		Iterator() : hash(0), scan(0) { }

		sf_forceinline bool operator==(const Iterator &rhs) const {
			return hash + scan == rhs.hash + rhs.scan;
		}
		sf_forceinline bool operator!=(const Iterator &rhs) const {
			return hash + scan != rhs.hash + rhs.scan;
		}

		Iterator &operator++();

		sf_forceinline uint32_t operator*() {
			sf_assert(scan > 0);
			return sf::hashReverse32(hash);
		}
	};

	rhset set;

	Iterator begin() const { return Iterator(&set); }
	Iterator end() const { return Iterator(); }

	UintSet();
	UintSet(const UintSet &rhs);
	UintSet(UintSet &&rhs);
	UintSet& operator=(const UintSet &rhs);
	UintSet& operator=(UintSet &&rhs);
	~UintSet();

	sf_forceinline uint32_t size() const { return set.size; }
	sf_forceinline uint32_t capacity() const { return set.capacity; }

	void insertDuplicate(uint32_t key);
	bool insertIfNew(uint32_t key);
	bool findOne(uint32_t key) const;
	uint32_t findAll(uint32_t key) const;

	bool removeOne(uint32_t key);
	uint32_t removeAll(uint32_t key);

	void clear();

	void reserve(uint32_t size)
	{
		if (size > set.capacity) {
			growImp(size);
		}
	}

	void growImp(uint32_t size);
};

}
