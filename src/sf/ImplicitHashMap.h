#pragma once

#include "Base.h"
#include "ext/rhmap.h"

namespace sf {

struct ImplicitHashMapBase
{
	rhmap map;
	void *data;
};

template <typename T, typename KeyFn>
struct ImplicitHashMap
{
	typedef T Entry;
	rhmap map;
	Entry *data;

	ImplicitHashMap()
	{
		memset(this, 0, sizeof(ImplicitHashMap));
	}

	ImplicitHashMap(const ImplicitHashMap &rhs)
	{
		memset(this, 0, sizeof(ImplicitHashMap));
		reserve(rhs.map.size);
		for (const Entry &entry : rhs) {
			insert(entry);
		}
	}

	ImplicitHashMap(ImplicitHashMap &&rhs)
	{
		data = rhs.data;
		map = rhs.map;
		rhmap_reset(&rhs.map);
		rhs.data = nullptr;
	}

	ImplicitHashMap& operator=(const ImplicitHashMap &rhs)
	{
		if (&rhs == this) return *this;
		clear();
		reserve(rhs.size());
		for (const Entry &entry : rhs) {
			insert(entry);
		}
		return *this;
	}

	ImplicitHashMap& operator=(ImplicitHashMap &&rhs)
	{
		if (&rhs == this) return *this;
		data = rhs.data;
		map = rhs.map;
		rhmap_reset(&rhs.map);
		rhs.data = nullptr;
		return *this;
	}

	~ImplicitHashMap()
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
	Entry &operator[](const KT &key)
	{
		uint32_t index;
		if (insertImp(key, index)) {
			new (&data[index]) Entry();
		}
		return data[index];
	}

	template <typename KT>
	Entry *find(const KT &key)
	{
		uint32_t index;
		uint32_t h = hash(key), scan = 0;
		while (rhmap_find(&map, h, &scan, &index)) {
			if (key == KeyFn()(data[index])) {
				return &data[index];
			}
		}
		return nullptr;
	}

	template <typename KT>
	sf_forceinline const Entry *find(const KT &key) const
	{
		return const_cast<ImplicitHashMap*>(this)->find(key);
	}

	template <typename KT>
	InsertResult<Entry> insert(const KT &key)
	{
		uint32_t index;
		bool inserted = insertImp(key, index);
		Entry *entry = &data[index];
		if (inserted) {
			new (&data[index]) Entry();
		}
		return { *entry, inserted };
	}

	InsertResult<Entry> insert(const Entry &entry)
	{
		uint32_t index;
		bool inserted = insertImp(KeyFn()((Entry&)entry), index);
		Entry *slot = &data[index];
		if (inserted) {
			new (slot) Entry(entry);
		}
		return { *slot, inserted };
	}

	InsertResult<Entry> insert(Entry &&entry)
	{
		uint32_t index;
		bool inserted = insertImp(KeyFn()((Entry&)entry), index);
		Entry *slot = &data[index];
		if (inserted) {
			new (slot) Entry(std::move(entry));
		}
		return { *slot, inserted };
	}

	InsertResult<Entry> insertOrAssign(const Entry &entry)
	{
		uint32_t index;
		bool inserted = insertImp(KeyFn()((Entry&)entry), index);
		Entry *slot = &data[index];
		if (!inserted) {
			slot->~Entry();
		}
		new (slot) Entry(entry);
		return { *slot, inserted };
	}

	template <typename KT>
	bool remove(const KT &key)
	{
		uint32_t index;
		uint32_t h = hash(key), scan = 0;
		while (rhmap_find(&map, h, &scan, &index)) {
			if (key == KeyFn()(data[index])) {
				rhmap_remove(&map, h, scan);
				if (index < map.size) {
					Entry &swap = data[map.size];
					rhmap_update_value(&map, hash(KeyFn()(swap)), map.size, index);
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
		bool res = remove(KeyFn()(*it));
		sf_assert(res);
		return it;
	}

protected:

	template <typename KT>
	bool insertImp(const KT &key, uint32_t &index)
	{
		if (map.size >= map.capacity) {
			growImp(128 / sizeof(Entry));
		}

		uint32_t h = hash(key), scan = 0;
		while (rhmap_find(&map, h, &scan, &index)) {
			if (key == KeyFn()(data[index])) {
				return false;
			}
		}

		index = map.size;
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

template <typename T, typename KeyFn> struct IsZeroInitializable<ImplicitHashMap<T, KeyFn>> { enum { value = 1 }; };

void initImplicitHashMapType(Type *t, const TypeInfo &info, Type *entryType, uint32_t (*hashFn)(void *inst));

template <typename T, typename KeyFn>
struct InitType<ImplicitHashMap<T, KeyFn>> {
	static void init(Type *t) {
		return initImplicitHashMapType(t, getTypeInfo<ImplicitHashMap<T, KeyFn>>(), typeOfRecursive<T>(),
		[](void *inst) { return hash(KeyFn()(*(T*)inst)); });
	}
};

}
