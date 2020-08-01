#pragma once

#include "sf/Base.h"
#include "sf/ext/rhmap.h"

namespace sv {

struct HashTableBase
{
	sf::MoveRangeFn moveRange;
	uint32_t numColumns;
	uint32_t elementSize;
	uint32_t size;
	uint32_t capacity;

	HashTableBase(sf::MoveRangeFn moveRange, uint32_t numColumns, uint32_t elementSize);

	void growImp(uint32_t minSize);

	void *insertImp(const void *elem);
	const void *findImp(uint32_t &scan, uint32_t columnIndex, uint32_t value) const;
	void removeImp(void *elem, uint32_t index);
	uint32_t removeAll(uint32_t columnIndex, uint32_t value);
};

template <typename T>
struct HashTable : HashTableBase
{
	T *data;
	rhmap maps[T::NumColumns];

	HashTable()
		: HashTableBase(&sf::moveRangeImp<T>, T::NumColumns, sizeof(T))
	{
	}

	~HashTable()
	{
		void *ptr = maps[0].entries;
		sf::destructRangeImp<T>(data, size);
		sf::memFree(ptr);
	}

	T *begin() { return data; }
	T *end() { return data + size; }
	const T *begin() const { return data; }
	const T *end() const { return data + size; }

	void insert(const T &t)
	{
		void *ptr = insertImp(&t);
		*(T*)ptr = t;
	}

	void insert(T &&t)
	{
		void *ptr = insertImp(&t);
		*(T*)ptr = std::move(t);
	}

	T *find(uint32_t &scan, uint32_t columnIndex, uint32_t value)
	{
		return (T*)findImp(scan, columnIndex, value);
	}

	const T *find(uint32_t &scan, uint32_t columnIndex, uint32_t value) const
	{
		return (const T*)findImp(scan, columnIndex, value);
	}

	void remove(T *row)
	{
		removeImp(row, row - data);
	}
};

void initHashTableType(sf::Type *t, const sf::TypeInfo &info, sf::Type *elementType);

}

namespace sf {

template <typename T>
struct InitType<sv::HashTable<T>> {
	static void init(Type *t) {
		return sv::initHashTableType(t, getTypeInfo<sv::HashTable<T>>(), typeOfRecursive<T>());
	}
};

}
