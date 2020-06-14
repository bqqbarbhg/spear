#pragma once

#include "sf/Base.h"
#include "sf/ext/rhmap.h"

namespace sv {

template <typename T> inline sf_forceinline uint64_t getIdImp(T &t) { return t.id.id; }

template <typename T, typename Id>
struct IdMap
{
	using Entry = T;

	rhmap map;
	Entry *data;

	IdMap()
	{
		memset(this, 0, sizeof(IdMap));
	}

	IdMap(const IdMap &rhs) = delete;

	IdMap(IdMap &&rhs)
	{
		data = rhs.data;
		map = rhs.map;
		rhmap_reset(&rhs.map);
		rhs.data = nullptr;
	}

	~IdMap()
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

	Entry *find(Id id)
	{
		uint32_t index;
		uint32_t h = sf::hash(id.id), scan = 0;
		while (rhmap_find(&map, h, &scan, &index)) {
			if (id.id == getIdImp(data[index])) {
				return &data[index];
			}
		}
		return nullptr;
	}

	sf_forceinline const Entry *find(Id id) const
	{
		return const_cast<IdMap<T, Id>*>(this)->find(id);
	}

	sf::InsertResult<Entry> insertUninit(const Id &id)
	{
		uint32_t index;
		if (map.size >= map.capacity) {
			growImp(128 / sizeof(Entry));
		}

		uint32_t h = sf::hash(id.id), scan = 0;
		while (rhmap_find(&map, h, &scan, &index)) {
			if (id.id == getIdImp(data[index])) {
				return { data[index], false };
			}
		}

		index = map.size;
		rhmap_insert(&map, h, scan, index);
		Entry &entry = data[index];
		return { entry, true };
	}

	bool remove(Id id)
	{
		uint32_t index;
		uint32_t h = sf::hash(id.id), scan = 0;
		while (rhmap_find(&map, h, &scan, &index)) {
			if (id.id == getIdImp(data[index])) {
				rhmap_remove(&map, h, scan);
				if (index < map.size) {
					Entry &swap = data[map.size];
					rhmap_update_value(&map, sf::hash(getIdImp(swap)), map.size, index);
					data[index].~Entry();
					new (&data[index]) Entry(std::move(swap));
				}
				data[map.size].~Entry();
				return true;
			}
		}
		return false;
	}

protected:

	void growImp(uint32_t size)
	{
		size_t count, allocSize;
		rhmap_grow(&map, &count, &allocSize, size, 0.8);

		void *newAlloc = sf::memAlloc(allocSize + count * sizeof(Entry));
		Entry *newData = (Entry*)((char*)newAlloc + allocSize);
		sf::moveRangeImp<Entry>(newData, data, map.size);
		data = newData;

		void *oldAlloc = rhmap_rehash(&map, count, allocSize, newAlloc);
		sf::memFree(oldAlloc);
	}
};

}
