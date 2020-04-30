#include "Map.h"

#define RHMAP_INLINE sf_inline
#include "sf/ext/rhmap.h"

struct MapEntity
{
	sf::Vec2i tile;
	Entity entity;
};

struct Map::Data
{
	sf::Array<MapEntity> entities;
	rhmap entityToMap;
	rhmap tileToMap;
};

Map::Map() : data(new Data())
{
}

Map::~Map()
{
	delete data;
}

sf::Vec2i Map::getTile(Entity e)
{
	uint32_t h = hash(e), scan = 0, index;
	while (rhmap_find_inline(&data->entityToMap, h, &scan, &index)) {
		MapEntity &me = data->entities[index];
		if (me.entity == e) return me.tile;
	}
	return { };
}

void Map::setTile(Entity e, const sf::Vec2i &tile)
{
	if (data->entityToMap.size == data->entityToMap.capacity) {
		size_t count, allocSize;
		rhmap_grow(&data->entityToMap, &count, &allocSize, 64, 0);
		char *memory = (char*)sf::memAlloc(allocSize * 2);
		void *oldMemory = rhmap_rehash(&data->entityToMap, count, allocSize, memory);
		rhmap_rehash(&data->tileToMap, count, allocSize, memory + allocSize);
		sf::memFree(oldMemory);
	}

	uint32_t h = hash(e), scan = 0, index;
	while (rhmap_find_inline(&data->entityToMap, h, &scan, &index)) {
		MapEntity &me = data->entities[index];
		if (me.entity == e) {
			if (me.tile == tile) return;

			uint32_t oldH = hash(me.tile);
			uint32_t newH = hash(tile);
			rhmap_find_value_inline(&data->tileToMap, oldH, &scan, index);
			rhmap_remove_inline(&data->tileToMap, oldH, scan);
			rhmap_insert_inline(&data->tileToMap, newH, 0, index);

			me.tile = tile;
			return;
		}
	}

	index = data->entities.size;
	MapEntity &me = data->entities.push();
	me.entity = e;
	me.tile = tile;

	uint32_t tileH = hash(tile);
	rhmap_insert_inline(&data->tileToMap, tileH, 0, index);
	rhmap_insert_inline(&data->entityToMap, h, scan, index);
}

void Map::getEntitiesInTile(const sf::Vec2i &tile, sf::Array<Entity> &entities)
{
	uint32_t h = hash(tile), scan = 0, index;
	while (rhmap_find_inline(&data->entityToMap, h, &scan, &index)) {
		MapEntity &me = data->entities[index];
		if (me.tile == tile) entities.push(me.entity);
	}
}

