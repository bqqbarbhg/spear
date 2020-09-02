#include "UintSet.h"

#define RHSET_INLINE sf_inline
#include "ext/rhset.h"

#include "sf/Reflection.h"

namespace sf {

UintSet::Iterator::Iterator(const rhset *beginFromSet)
	: set(beginFromSet)
{
	hash = scan = 0;
	rhset_next_inline(set, &hash, &scan);
}

UintSet::Iterator &UintSet::Iterator::operator++()
{
	if (!rhset_next_inline(set, &hash, &scan)) {
		hash = scan = 0;
	}
	return *this;
}

UintSet::UintSet()
{
	rhset_init_inline(&set);
}

UintSet::UintSet(const UintSet &rhs)
{
	rhset_init_inline(&set);
	growImp(rhs.set.size);
	uint32_t hash = 0, scan = 0;
	while (rhset_next_inline(&rhs.set, &hash, &scan)) {
		rhset_insert_inline(&set, hash, scan - 1);
	}
}

UintSet::UintSet(UintSet &&rhs)
	: set(rhs.set)
{
	rhset_reset_inline(&rhs.set);
}

UintSet& UintSet::operator=(const UintSet &rhs)
{
	if (&rhs == this) return *this;
	clear();
	reserve(rhs.set.size);
	uint32_t hash = 0, scan = 0;
	while (rhset_next_inline(&rhs.set, &hash, &scan)) {
		rhset_insert_inline(&set, hash, scan - 1);
	}
	return *this;
}

UintSet& UintSet::operator=(UintSet &&rhs)
{
	if (&rhs == this) return *this;
	memFree(set.entries);
	set = rhs.set;
	rhset_reset_inline(&rhs.set);
	return *this;
}

UintSet::~UintSet()
{
	void *ptr = rhset_reset_inline(&set);
	memFree(ptr);
}

void UintSet::clear()
{
	rhset_clear_inline(&set);
}

void UintSet::growImp(uint32_t size)
{
	size_t count, allocSize;
	rhset_grow_inline(&set, &count, &allocSize, size, 0.8);

	void *newAlloc = memAlloc(allocSize);
	void *oldAlloc = rhset_rehash_inline(&set, count, allocSize, newAlloc);
	memFree(oldAlloc);
}

void UintSet::insertDuplicate(uint32_t key)
{
	if (set.size == set.capacity) growImp(10);
	uint32_t hash = sf::hash(key);
	rhset_insert_inline(&set, hash, 0);
}

bool UintSet::insertIfNew(uint32_t key)
{
	if (set.size == set.capacity) growImp(10);
	uint32_t hash = sf::hash(key);

	uint32_t scan = 0;
	if (rhset_find_inline(&set, hash, &scan)) {
		return false;
	}

	rhset_insert_inline(&set, hash, 0);
	return true;
}

bool UintSet::findOne(uint32_t key) const
{
	uint32_t hash = sf::hash(key);
	uint32_t scan = 0;
	if (rhset_find_inline(&set, hash, &scan)) {
		return true;
	} else {
		return false;
	}
}

uint32_t UintSet::findAll(uint32_t key) const
{
	uint32_t num = 0;
	uint32_t hash = sf::hash(key);
	uint32_t scan = 0;
	while (rhset_find_inline(&set, hash, &scan)) {
		num++;
	}
	return num;
}

bool UintSet::removeOne(uint32_t key)
{
	uint32_t hash = sf::hash(key);
	uint32_t scan = 0;
	if (rhset_find_inline(&set, hash, &scan)) {
		rhset_remove_inline(&set, hash, scan);
		return true;
	}
	return false;
}

uint32_t UintSet::removeAll(uint32_t key)
{
	uint32_t num = 0;
	uint32_t hash = sf::hash(key);
	uint32_t scan = 0;
	while (rhset_find_inline(&set, hash, &scan)) {
		rhset_remove_inline(&set, hash, scan);
		scan--;
		num++;
	}
	return num;
}

struct UintSetType : Type
{
	UintSetType()
		: Type("sf::UintSet", getTypeInfo<UintSet>(), HasArray|HasArrayResize)
	{
		elementType = typeOfRecursive<uint32_t>();
	}

	virtual VoidSlice instGetArray(void *inst, sf::Array<char> *scratch) override
	{
		if (!scratch) return { };

		UintSet &set = *(UintSet*)inst;
		scratch->clear();
		uint32_t *data = (uint32_t*)scratch->pushUninit(sizeof(uint32_t) * set.set.size);

		uint32_t *dst = data;
		uint32_t hash = 0, scan = 0;
		while (rhset_next_inline(&set.set, &hash, &scan)) {
			*dst = sf::hashReverse32(hash);
			dst++;
		}

		return { data, set.set.size };
	}

	virtual VoidSlice instArrayReserve(void *inst, size_t size, sf::Array<char> *scratch) override
	{
		if (!scratch) return { };

		scratch->clear();
		uint32_t *data = (uint32_t*)scratch->pushUninit(sizeof(uint32_t) * size);
		return { data, size };
	}

	virtual void instArrayResize(void *inst, size_t size, VoidSlice elements) override
	{
		sf::Slice<uint32_t> data = elements.cast<uint32_t>();
		UintSet &set = *(UintSet*)inst;
		set.reserve((uint32_t)size);

		for (uint32_t key : data) {
			uint32_t hash = sf::hash(key);
			rhset_insert_inline(&set.set, hash, 0);
		}
	}
};

template<> void initType<UintSet>(Type *t) { new (t) UintSetType(); }

}
