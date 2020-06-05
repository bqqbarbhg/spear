#pragma once

#include "Base.h"
#include "ext/rhmap.h"

namespace sf {

struct HashSetBase
{
	rhmap map;
	void *data;
};

template <typename T>
struct HashSet
{
	typedef T Entry;
	rhmap map;
	Entry *data;

	HashSet()
	{
		memset(this, 0, sizeof(HashSet));
	}

	HashSet(const HashSet &rhs)
	{
		memset(this, 0, sizeof(HashSet));
		reserve(rhs.map.size);
		for (const Entry &entry : rhs) {
			insert(entry);
		}
	}

	HashSet(HashSet &&rhs)
	{
		data = rhs.data;
		map = rhs.map;
		rhmap_reset(&rhs.map);
		rhs.data = nullptr;
	}

	~HashSet()
	{
		destructRangeImp<Entry>(data, map.size);
		void *oldAlloc = rhmap_reset(&map);
		memFree(oldAlloc);
	}

	sf_forceinline uint32_t size() const { return map.size; }
	sf_forceinline uint32_t capacity() const { return map.capacity; }
	sf_forceinline Entry *begin() { return data; }
	sf_forceinline Entry *end() { return data + map.size; }
	sf_forceinline const Entry *begin() const { return data; }
	sf_forceinline const Entry *end() const { return data + map.size; }

	void clear()
	{
		rhmap_clear(&map);
	}

	void reserve(uint32_t size)
	{
		if (size > map.capacity) {
			growImp(size);
		}
	}

	template <typename KT>
	Entry *find(const KT &key)
	{
		uint32_t index;
		uint32_t h = hash(key), scan = 0;
		while (rhmap_find(&map, h, &scan, &index)) {
			if (key == data[index]) {
				return &data[index];
			}
		}
		return nullptr;
	}

	template <typename KT>
	sf_forceinline const Entry *find(const KT &key) const
	{
		return const_cast<HashSet<T>*>(this)->find(key);
	}

	template <typename KT>
	InsertResult<Entry> insert(const KT &key)
	{
		uint32_t index;
		bool inserted = insertImp(key, index);
		Entry *entry = &data[index];
		return { *entry, inserted };
	}

	template <typename KT>
	bool remove(const KT &key)
	{
		uint32_t index;
		uint32_t h = hash(key), scan = 0;
		while (rhmap_find(&map, h, &scan, &index)) {
			if (key == data[index]) {
				rhmap_remove(&map, h, scan);
				if (index < map.size) {
					Entry &swap = data[map.size];
					rhmap_update_value(&map, hash(swap), map.size, index);
					data[index].~Entry();
					new (&data[index]) Entry(std::move(swap));
				}
				data[map.size].~Entry();
				return true;
			}
		}
		return false;
	}

	Entry *removeAt(Entry *it)
	{
		bool res = remove(it->key);
		sf_assert(res);
		return it;
	}

	sf::Slice<T> slice() { return sf::Slice<T>(data, map.size); }
	sf::Slice<const T> slice() const { return sf::Slice<const T>(data, map.size); }

protected:

	template <typename KT>
	bool insertImp(const KT &key, uint32_t &index)
	{
		if (map.size >= map.capacity) {
			growImp(128 / sizeof(Entry));
		}

		uint32_t h = hash(key), scan = 0;
		while (rhmap_find(&map, h, &scan, &index)) {
			if (key == data[index]) {
				return false;
			}
		}

		index = map.size;
		new (&data[index]) T(key);
		rhmap_insert(&map, h, scan, index);
		return true;
	}

	void growImp(uint32_t size)
	{
		size_t count, allocSize;
		rhmap_grow(&map, &count, &allocSize, size, 0.8);

		void *newAlloc = memAlloc(allocSize + count * sizeof(Entry));
		Entry *newData = (Entry*)((char*)newAlloc + allocSize);
		moveRangeImp<Entry>(newData, data, map.size);
		data = newData;

		void *oldAlloc = rhmap_rehash(&map, count, allocSize, newAlloc);
		memFree(oldAlloc);
	}
};

template <typename T> struct IsZeroInitializable<HashSet<T>> { enum { value = 1 }; };

void initHashSetType(Type *t, const TypeInfo &info, Type *kvType, uint32_t (*hashFn)(void *inst));

template <typename T>
struct InitType<HashSet<T>> {
	static void init(Type *t) {
		return initHashSetType(t, getTypeInfo<HashSet<T>>(), typeOfRecursive<T>(),
		[](void *inst) { return hash(*(T*)inst); });
	}
};


}
