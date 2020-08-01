#include "HashTable.h"

#define RHMAP_INLINE static sf_forceinline
#define RHMAP_ASSERT(cond) sf_assert(cond)
#include "sf/ext/rhmap.h"

#include "sf/Reflection.h"

namespace sv {

sf_inline uint32_t columnHash(uint32_t value)
{
	return sf::hash(value);
}

sf_inline void **getImpDataPtr(const HashTableBase &table)
{
	return (void**)((char*)&table + sizeof(HashTableBase));
}

sf_inline rhmap *getImpMaps(const HashTableBase &table)
{
	return (rhmap*)((char*)&table + (sizeof(HashTableBase) + sizeof(void*)));
}

HashTableBase::HashTableBase(sf::MoveRangeFn moveRange, uint32_t numColumns, uint32_t elementSize)
	: moveRange(moveRange), numColumns(numColumns), elementSize(elementSize), size(0), capacity(0)
{
	memset(this + 1, 0, sizeof(void*) + sizeof(rhmap) * numColumns);
}

void HashTableBase::growImp(uint32_t minSize)
{
	// All the maps are the same size
	rhmap *maps = getImpMaps(*this);
	void **dataPtr = getImpDataPtr(*this);

	size_t count, allocSize;
	rhmap_grow_inline(maps, &count, &allocSize, minSize, 0.0);

	void *newAlloc = sf::memAlloc(allocSize*numColumns + count*elementSize);
	void *newData = ((char*)newAlloc + allocSize*numColumns);

	moveRange(newData, *dataPtr, size);

	char *rhPtr = (char*)newAlloc;
	for (uint32_t i = 0; i < numColumns; i++) {
		rhmap_rehash_inline(&maps[i], count, allocSize, rhPtr);
		rhPtr += allocSize;
	}

	*dataPtr = newData;
	capacity = (uint32_t)count;
}

void *HashTableBase::insertImp(const void *elem)
{
	if (size == capacity) growImp(16);

	rhmap *maps = getImpMaps(*this);
	void *data = *getImpDataPtr(*this);

	uint32_t index = size++;

	const uint32_t *keys = (const uint32_t*)elem;
	for (uint32_t i = 0; i < numColumns; i++) {
		uint32_t hash = columnHash(keys[i]);
		rhmap_insert_inline(&maps[i], hash, 0, index);
	}

	return (char*)data + index * elementSize;
}

const void *HashTableBase::findImp(uint32_t &scan, uint32_t columnIndex, uint32_t value) const
{
	rhmap *maps = getImpMaps(*this);
	void *data = *getImpDataPtr(*this);
	rhmap &map = maps[columnIndex];

	uint32_t hash = columnHash(value);
	uint32_t index;
	if (rhmap_find_inline(&map, hash, &scan, &index)) {
		return (char*)data + index * elementSize;
	} else {
		return nullptr;
	}
}

void HashTableBase::removeImp(void *elem, uint32_t index)
{
	size--;
	const uint32_t *keys = (const uint32_t*)elem;
	rhmap *maps = getImpMaps(*this);
	void *data = *getImpDataPtr(*this);

	if (index < size) {

		uint32_t *swapKeys = (uint32_t*)((char*)data + size * elementSize);
		for (uint32_t i = 0; i < numColumns; i++) {
			uint32_t hash = columnHash(keys[i]);
			uint32_t swapHash = columnHash(swapKeys[i]);
			uint32_t scan = 0;
			rhmap_find_value_inline(&maps[i], hash, &scan, index);
			rhmap_remove_inline(&maps[i], hash, scan);
			rhmap_update_value_inline(&maps[i], swapHash, size, index);
		}

		moveRange(swapKeys, elem, 1);

	} else {

		for (uint32_t i = 0; i < numColumns; i++) {
			uint32_t hash = columnHash(keys[i]);
			uint32_t scan = 0;
			rhmap_find_value_inline(&maps[i], hash, &scan, index);
			rhmap_remove_inline(&maps[i], hash, scan);
		}

	}
}

uint32_t HashTableBase::removeAll(uint32_t columnIndex, uint32_t value)
{
	void* rows[64];
	void *data = *getImpDataPtr(*this);
	uint32_t numRemoved = 0;

	void *row;
	do {
		uint32_t numRows = 0;
		uint32_t scan = 0;
		for (;;) {
			row = (void*)findImp(scan, columnIndex, value);
			if (!row) break;
			rows[numRows++] = row;
		}

		for (uint32_t i = 0; i < numRows; i++) {
			removeImp(rows[i], (uint32_t)(((char*)rows[i] - (char*)data) / elementSize));
		}

		numRemoved += numRows;
	} while (row);

	return numRemoved;
}

struct HashTableType final : sf::Type
{
	uint32_t (*hashFn)(void *inst);

	HashTableType(const sf::TypeInfo &info, sf::Type *elemType)
		: Type("sv::HashTable", info, HasArray | HasArrayResize)
	{
		elementType = elemType;
	}

	virtual void getName(sf::StringBuf &buf)
	{
		buf.append("sf::HashTable<");
		elementType->getName(buf);
		buf.append(">");
	}

	virtual sf::VoidSlice instGetArray(void *inst)
	{
		HashTableBase *tab = (HashTableBase*)inst;
		return { *getImpDataPtr(*tab), tab->size };
	}

	virtual sf::VoidSlice instArrayReserve(void *inst, size_t size)
	{
		HashTableBase *tab = (HashTableBase*)inst;
		tab->growImp((uint32_t)size);
		return { *getImpDataPtr(*tab), tab->size };
	}

	virtual void instArrayResize(void *inst, size_t size)
	{
		HashTableBase *tab = (HashTableBase*)inst;
		uint32_t elemSize = tab->elementSize;
		sf_assert(tab->capacity >= size);
		void *data = *getImpDataPtr(*tab);
		if (tab->size > size) {
			uint32_t numToRemove = (uint32_t)(tab->size - size);
			uint32_t index = tab->size - 1;
			char *elem = (char*)data + index * elemSize;
			for (uint32_t i = 0; i < numToRemove; i++) {
				tab->removeImp(elem, index);
				elem -= elemSize;
				index -= 1;
			}
			elementType->info.destructRange((char*)data + size*elemSize, numToRemove);
		}

		char *elem = (char*)data + tab->size * elemSize;
		rhmap *maps = getImpMaps(*tab);
		for (uint32_t index = tab->size; index < size; index++) {
			const uint32_t *keys = (const uint32_t*)elem;
			for (uint32_t i = 0; i < tab->numColumns; i++) {
				uint32_t hash = columnHash(keys[i]);
				rhmap_insert_inline(&maps[i], hash, 0, index);
			}
			elem += tab->elementSize;
		}
		tab->size = (uint32_t)size;
	}
};


void initHashTableType(sf::Type *t, const sf::TypeInfo &info, sf::Type *elementType)
{
	new (t) HashTableType(info, elementType);
}

}
