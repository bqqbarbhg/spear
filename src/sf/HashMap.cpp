#include "HashMap.h"
#include "Reflection.h"

namespace sf {

struct KeyValType final : Type
{
	Field pair[2];
	ConstructRangeFn ctor;
	MoveRangeFn move;
	DestructRangeFn dtor;

	KeyValType(size_t kvSize)
		: Type("KeyVal", kvSize, HasFields)
	{
	}

	virtual void getName(sf::StringBuf &buf)
	{
		buf.append("KeyVal<");
		pair[0].type->getName(buf);
		buf.append(", ");
		pair[1].type->getName(buf);
		buf.append(">");
	}

	virtual void instConstruct(void *inst, size_t num)
	{
		ctor(inst, num);
	}

	virtual void instMove(void *dst, void *src, size_t num)
	{
		move(dst, src, num);
	}

	virtual void instDestruct(void *inst, size_t num)
	{
		dtor(inst, num);
	}
};

void initKeyValType(Type *tt, Type *keyType, Type *valType, size_t valOffset, size_t kvSize, ConstructRangeFn ctor, MoveRangeFn move, DestructRangeFn dtor)
{
	KeyValType *t = (KeyValType*)tt;
	new (t) KeyValType(kvSize);
	t->pair[0].name = "key";
	t->pair[0].offset = 0;
	t->pair[0].size = keyType->size;
	t->pair[0].type = keyType;
	t->pair[1].name = "val";
	t->pair[1].offset = (uint32_t)valOffset;
	t->pair[1].size = valType->size;
	t->pair[1].type = valType;
	t->fields = t->pair;
	t->ctor = ctor;
	t->move = move;
	t->dtor = dtor;
	t->flags |= keyType->flags & valType->flags & (Type::IsPod|Type::CompactString);
}

struct HashMapType final : Type
{
	uint32_t (*hashFn)(void *inst);

	HashMapType(Type *kvType, uint32_t (*hashFn)(void *inst))
		: Type("HashMap", sizeof(HashMapBase), HasArray)
		, hashFn(hashFn)
	{
		elementType = kvType;
	}

	virtual void getName(sf::StringBuf &buf)
	{
		buf.append("HashMap<");
		elementType->fields[0].type->getName(buf);
		buf.append(", ");
		elementType->fields[1].type->getName(buf);
		buf.append(">");
	}

	virtual VoidSlice instGetArray(void *inst)
	{
		HashMapBase *map = (HashMapBase*)inst;
		return { map->data, map->map.size };
	}

	virtual VoidSlice instArrayReserve(void *inst, size_t size)
	{
		uint32_t kvSize = elementType->size;
		KeyValType *kvType = (KeyValType*)elementType;

		HashMapBase *map = (HashMapBase*)inst;
		if (map->map.capacity < size) {
			size_t count, allocSize;
			rhmap_grow(&map->map, &count, &allocSize, size, 0.8);

			void *newAlloc = memAlloc(allocSize + count * kvSize);
			void *newData = (char*)newAlloc + allocSize;
			if (map->map.size) {
				kvType->move(newData, map->data, map->map.size);
			}
			map->data = newData;

			void *oldAlloc = rhmap_rehash(&map->map, count, allocSize, newAlloc);
			memFree(oldAlloc);
		}

		if (size > map->map.size) {
			char *base = (char*)map->data + map->map.size * kvSize;
			kvType->ctor(base, size - map->map.size);
		}

		return { map->data, map->map.capacity };
	}

	virtual void instArrayResize(void *inst, size_t size)
	{
		uint32_t kvSize = elementType->size;

		HashMapBase *map = (HashMapBase*)inst;
		sf_assert(map->map.capacity >= size);
		if (map->map.size > size) {
			char *base = (char*)map->data + map->map.size * kvSize;
			char *ptr = base;
			for (uint32_t i = map->map.size; i < size; i++) {
				rhmap_iter iter = { &map->map, hashFn(ptr) };
				rhmap_find_value(&iter, i);
				rhmap_remove(&iter);
				ptr += kvSize;
			}
			elementType->instDestruct(base, map->map.size - size);
		}
		char *ptr = (char*)map->data + map->map.size * kvSize;
		for (uint32_t i = map->map.size; i < size; i++) {
			rhmap_iter iter = { &map->map, hashFn(ptr) };
			rhmap_insert(&iter, i);
			ptr += kvSize;
		}
	}
};

void initHashMapType(Type *t, Type *kvType, uint32_t (*hashFn)(void *inst))
{
	new (t) HashMapType(kvType, hashFn);
}

}
