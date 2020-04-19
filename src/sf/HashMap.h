#pragma once

#include "Base.h"
#include "ext/rhmap.h"

namespace sf {

template <typename K, typename V>
struct KeyVal
{
	K key;
	V val;
};

template <typename T>
struct InsertResult
{
	T &entry;
	bool inserted;
};

struct HashMapBase
{
	rhmap map;
	void *data;
};

template <typename K, typename V>
struct HashMap
{
	typedef KeyVal<K, V> Entry;
	rhmap map;
	Entry *data;

	HashMap()
	{
		memset(this, 0, sizeof(HashMap));
	}

	HashMap(const HashMap &rhs)
	{
		memset(this, 0, sizeof(HashMap));
		reserve(rhs.map.size);
		for (Entry &entry : rhs) {
			insert(entry.key, entry.val);
		}
	}

	HashMap(HashMap &&rhs)
	{
		data = rhs.data;
		map = rhs.map;
		rhmap_reset(&rhs.map);
		rhs.data = nullptr;
	}

	~HashMap()
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
	V &operator[](const KT &key)
	{
		uint32_t index;
		if (insertImp(key, index)) {
			new (&data[index].val) V();
		}
		return data[index].val;
	}

	template <typename KT>
	Entry *find(const KT &key)
	{
		uint32_t index;
		rhmap_iter iter = { &map, hash(key) };
		while (rhmap_find(&iter, &index)) {
			if (key == data[index].key) {
				return &data[index];
			}
		}
		return nullptr;
	}

	template <typename KT>
	sf_forceinline const Entry *find(const KT &key) const
	{
		return const_cast<HashMap<K, V>*>(this)->find(key);
	}

	template <typename KT>
	InsertResult<Entry> insert(const KT &key)
	{
		uint32_t index;
		bool inserted = insertImp(key, index);
		Entry *entry = &data[index];
		if (inserted) {
			new (&entry->val) V();
		}
		return { *entry, inserted };
	}

	template <typename KT>
	InsertResult<Entry> insert(const KT &key, const V &value)
	{
		uint32_t index;
		bool inserted = insertImp(key, index);
		Entry *entry = &data[index];
		if (inserted) {
			new (&entry->val) V(value);
		}
		return { *entry, inserted };
	}

	template <typename KT>
	InsertResult<Entry> insertOrAssign(const KT &key, const V &value)
	{
		uint32_t index;
		bool inserted = insertImp(key, index);
		Entry *entry = &data[index];
		if (!inserted) {
			entry->val.~V();
		}
		new (&entry->val) V(value);
		return { entry, inserted };
	}

	template <typename KT>
	bool remove(const KT &key)
	{
		uint32_t index;
		rhmap_iter iter = { &map, hash(key) };
		while (rhmap_find(&iter, &index)) {
			if (key == data[index].key) {
				rhmap_remove(&iter);
				if (index < map.size) {
					Entry &swap = data[map.size];
					rhmap_update(&map, hash(swap.key), map.size, index);
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

protected:

	template <typename KT>
	bool insertImp(const KT &key, uint32_t &index)
	{
		if (map.size >= map.capacity) {
			growImp(128 / sizeof(Entry));
		}

		rhmap_iter iter = { &map, hash(key) };
		while (rhmap_find(&iter, &index)) {
			if (key == data[index].key) {
				return false;
			}
		}

		index = map.size;
		new (&data[index].key) K(key);
		rhmap_insert(&iter, index);
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

template <typename K, typename V> struct IsZeroInitializable<HashMap<K, V>> { enum { value = 1 }; };

Type *initKeyValType(Type *keyType, Type *valType, size_t valOffset, size_t kvSize, ConstructRangeFn ctor, MoveRangeFn move, DestructRangeFn dtor);
Type *initHashMapType(Type *kvType, uint32_t (*hashFn)(void *inst));

template <typename K, typename V>
struct InitType<KeyVal<K, V>> {
	static Type *init() { using KV = KeyVal<K, V>; return initKeyValType(typeOf<K>(), typeOf<V>(), offsetof(KV, val), sizeof(KV),
		&constructRangeImp<KV>, &moveRangeImp<KV>, &destructRangeImp<KV>); }
};

template <typename K, typename V>
struct InitType<HashMap<K, V>> {
	static Type *init() {
		return initHashMapType(typeOf<KeyVal<K, V>>(),
		[](void *inst) { return hash(*(K*)inst); });
	}
};


}
