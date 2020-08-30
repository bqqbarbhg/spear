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
		for (const Entry &entry : rhs) {
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

	HashMap& operator=(const HashMap &rhs)
	{
		if (&rhs == this) return *this;
		clear();
		reserve(rhs.size);
		for (const auto &pair : rhs) {
			insert(pair.key, pair.value);
		}
		return *this;
	}

	HashMap& operator=(HashMap &&rhs)
	{
		if (&rhs == this) return *this;
		data = rhs.data;
		map = rhs.map;
		rhmap_reset(&rhs.map);
		rhs.data = nullptr;
		return *this;
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
		uint32_t h = hash(key), scan = 0;
		while (rhmap_find(&map, h, &scan, &index)) {
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
	V *findValue(const KT &key) {
		Entry *entry = find(key);
		return entry ? &entry->val : NULL;
	}

	template <typename KT>
	const V *findValue(const KT &key) const {
		const Entry *entry = find(key);
		return entry ? &entry->val : NULL;
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
		uint32_t h = hash(key), scan = 0;
		while (rhmap_find(&map, h, &scan, &index)) {
			if (key == data[index].key) {
				rhmap_remove(&map, h, scan);
				if (index < map.size) {
					Entry &swap = data[map.size];
					rhmap_update_value(&map, hash(swap.key), map.size, index);
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

		uint32_t h = hash(key), scan = 0;
		while (rhmap_find(&map, h, &scan, &index)) {
			if (key == data[index].key) {
				return false;
			}
		}

		index = map.size;
		new (&data[index].key) K(key);
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

template <typename K, typename V> struct IsZeroInitializable<HashMap<K, V>> { enum { value = 1 }; };

void initKeyValType(Type *t, const TypeInfo &info, Type *keyType, Type *valType, size_t valOffset);
void initHashMapType(Type *t, const TypeInfo &info, Type *kvType, uint32_t (*hashFn)(void *inst));

template <typename K, typename V>
struct InitType<KeyVal<K, V>> {
	static void init(Type *t) {
		using KV = KeyVal<K, V>;
		initKeyValType(t, getTypeInfo<KV>(), typeOfRecursive<K>(), typeOfRecursive<V>(), offsetof(KV, val));
	}
};

template <typename K, typename V>
struct InitType<HashMap<K, V>> {
	static void init(Type *t) {
		return initHashMapType(t, getTypeInfo<HashMap<K, V>>(), typeOfRecursive<KeyVal<K, V>>(),
		[](void *inst) { return hash(*(K*)inst); });
	}
};


}
