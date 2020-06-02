#include "GameState.h"

#include "sf/Reflection.h"
#include "game/server/Event.h"
#include "game/server/Action.h"

#include "sf/Mutex.h"

namespace sv {

bool TileType::operator==(const TileType &rhs) const
{
	if (floorName != rhs.floorName) return false;
	if (tileName != rhs.tileName) return false;
	if (floor != rhs.floor) return false;
	if (wall != rhs.wall) return false;
	return true;
}

uint32_t hash(const TileType &t)
{
	uint32_t h = 0;
	h = sf::hashCombine(h, sf::hash(t.floorName));
	h = sf::hashCombine(h, sf::hash(t.tileName));
	h = sf::hashCombine(h, sf::hash(t.floor));
	h = sf::hashCombine(h, sf::hash(t.wall));
	return h;
}

struct CardTypeCache
{
	sf::HashMap<sf::Symbol, sf::Box<CardType>> map;
	sf::Mutex mutex;
};

static CardTypeCache g_cardTypeCache;

void Card::refresh()
{
	if (!type) return;
	sf::MutexGuard mg(g_cardTypeCache.mutex);
	auto res = g_cardTypeCache.map.insert(type->name, std::move(type));
	type = res.entry.val;
}

sf_inline void resolveChunk(sf::Vec2i &chunkI, sf::Vec2i &tileI, const sf::Vec2i &pos)
{
	chunkI = { pos.x >> (int32_t)MapChunk::SizeLog2, pos.y >> (int32_t)MapChunk::SizeLog2 };
	tileI = { pos.x & ((int32_t)MapChunk::Size - 1), pos.y & ((int32_t)MapChunk::Size - 1) };
}

sf::Vec2i Map::setTile(const sf::Vec2i &pos, TileId tileId)
{
	sf::Vec2i chunkI, tileI;
	resolveChunk(chunkI, tileI, pos);
	auto result = chunks.insert(chunkI);
	MapChunk &chunk = result.entry.val;
	TileId &dst = chunk.tiles[tileI.y * MapChunk::Size + tileI.x];
	if (dst == 0 && tileId != 0) chunk.numNonZeroTiles++;
	if (dst != 0 && tileId == 0) chunk.numNonZeroTiles--;
	dst = tileId;
	if (chunk.numNonZeroTiles == 0) {
		chunks.removeAt(&result.entry);
	}
	return chunkI;
}

bool Map::canStandOn(const sf::Vec2i &pos) const
{
	const TileType &type = tileTypes[getTile(pos)];
	return type.floor && !type.wall;
}

TileId Map::getTile(const sf::Vec2i &pos) const
{
	sf::Vec2i chunkI, tileI;
	resolveChunk(chunkI, tileI, pos);
	auto it = chunks.find(chunkI);
	if (!it) return 0;
	return it->val.tiles[tileI.y * MapChunk::Size + tileI.x];
}

void EntityTileMap::impGrow(size_t minSize)
{
	size_t count, allocSize;
	rhmap_grow(&map, &count, &allocSize, minSize, 0);
	rhmap_rehash(&map, count, allocSize, sf::memAlloc(allocSize));
}

EntityTileMap::EntityTileMap()
{
	rhmap_init(&map);
}

EntityTileMap::EntityTileMap(const EntityTileMap &rhs)
{
	rhmap_init(&map);
	if (rhs.map.size > 0) impGrow(rhs.map.size);
	
	uint32_t hash, scan, value;
	while (rhmap_next(&rhs.map, &hash, &scan, &value)) {
		rhmap_insert(&map, hash, scan, value);
	}
}

EntityTileMap::EntityTileMap(EntityTileMap &&rhs)
{
	map = rhs.map;
	rhmap_reset(&rhs.map);
}

EntityTileMap &EntityTileMap::operator=(const EntityTileMap &rhs)
{
	if (&rhs == this) return *this;
	sf::memFree(rhmap_reset(&map));
	if (rhs.map.size > 0) impGrow(rhs.map.size);
	
	uint32_t hash, scan, value;
	while (rhmap_next(&rhs.map, &hash, &scan, &value)) {
		rhmap_insert(&map, hash, scan, value);
	}
	return *this;
}

EntityTileMap &EntityTileMap::operator=(EntityTileMap &&rhs)
{
	if (&rhs == this) return *this;
	map = rhs.map;
	rhmap_reset(&rhs.map);
	return *this;
}

EntityTileMap::~EntityTileMap()
{
	sf::memFree(rhmap_reset(&map));
}

void EntityTileMap::add(EntityId entity, const sf::Vec2i &newPos)
{
	if (map.size == map.capacity) impGrow(16);
	uint32_t newHash = sf::hash(newPos);
	rhmap_insert(&map, newHash, 0, entity);
}

void EntityTileMap::update(EntityId entity, const sf::Vec2i &oldPos, const sf::Vec2i &newPos)
{
	uint32_t oldHash = sf::hash(oldPos);
	uint32_t newHash = sf::hash(newPos);
	uint32_t scan = 0;
	rhmap_find_value(&map, oldHash, &scan, entity);
	rhmap_remove(&map, oldHash, scan);
	rhmap_insert(&map, newHash, 0, entity);
}

void EntityTileMap::remove(EntityId entity, const sf::Vec2i &oldPos)
{
	uint32_t oldHash = sf::hash(oldPos);
	uint32_t scan = 0;
	rhmap_find_value(&map, oldHash, &scan, entity);
	rhmap_remove(&map, oldHash, scan);
}

void EntityTileMap::reserve(size_t size)
{
	if (map.size + size > map.capacity) impGrow(size);
}

void EntityTileMap::clear()
{
	rhmap_clear(&map);
}

void State::refreshEntityTileMap()
{
	entityTileMap.clear();
	entityTileMap.reserve(entities.size);

	EntityId id = 0;
	for (auto &entity : entities) {
		if (entity) {
			entityTileMap.add(id, entity->position);
		}
		id++;
	}
}

void State::initEntity(EntityId entity, sf::Box<Entity> data)
{
	while (entity >= entities.size) entities.push();
	sf_assert(!entities[entity]);
	entityTileMap.add(entity, data->position);
	entities[entity] = std::move(data);
}

void State::destroyEntity(EntityId entity)
{
	Entity *data = entities[entity];
	uint32_t oldHash = sf::hash(data->position);
	entityTileMap.remove(entity, data->position);
	entities[entity].reset();
}

void State::setEntityPosition(EntityId entity, const sf::Vec2i &pos)
{
	Entity *data = entities[entity];
	if (pos == data->position) return;
	entityTileMap.update(entity, data->position, pos);
	data->position = pos;
}

void State::getEntitiesOnTile(sf::Array<Entity*> &dst, const sf::Vec2i &pos) const
{
	uint32_t scan = 0, id;
	uint32_t hash = sf::hash(pos);
	while (rhmap_find(&entityTileMap.map, hash, &scan, &id)) {
		Entity *entity = entities[id];
		if (entity->position == pos) {
			dst.push(entity);
		}
	}
}

bool State::canStandOn(const sf::Vec2i &pos) const
{
	if (!map.canStandOn(pos)) return false;
	sf::SmallArray<Entity*, 16> entitiesOnTile;
	getEntitiesOnTile(entitiesOnTile, pos);
	for (Entity *entity : entitiesOnTile) {
		if (entity->blocksTile) return false;
	}
	return true;
}

void State::applyEvent(Event *event)
{
	if (auto e = event->as<EventMove>()) {
		setEntityPosition(e->entity, e->position);
	} else if (auto e = event->as<EventSpawn>()) {
		sf_assert(e->data->id != 0);
		initEntity(e->data->id, e->data);
	} else if (auto e = event->as<EventDestroy>()) {
		destroyEntity(e->entity);
	} else if (auto e = event->as<EventUpdateTileType>()) {
		while (map.tileTypes.size <= e->index) map.tileTypes.push();
		map.tileTypes[e->index] = e->tileType;
	} else if (auto e = event->as<EventUpdateChunk>()) {
		map.chunks[e->position] = e->chunk;
	}
}

bool State::applyAction(Action *action, sf::Array<sf::Box<Event>> &events, sf::StringBuf &error)
{
	if (auto move = action->as<ActionMove>()) {

		if (!canStandOn(move->position)) {
			error.format("Tile (%d,%d) is not free", move->position.x, move->position.y);
			return false;
		}

		auto event = sf::box<EventMove>();
		event->entity = move->entity;
		event->position = move->position;
		events.push(event);
		return true;

	} else {
		sf_failf("Unexpected action type: %u", action->type);
		return false;
	}
}

}

namespace sf {

template<> void initType<sv::TileType>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::TileType, floorName),
		sf_field(sv::TileType, tileName),
		sf_field(sv::TileType, floor),
		sf_field(sv::TileType, wall),
	};
	sf_struct(t, sv::TileType, fields);
}

template<> void initType<sv::CardType>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::CardType, id),
		sf_field(sv::CardType, image),
		sf_field(sv::CardType, name),
		sf_field(sv::CardType, description),
	};
	sf_struct(t, sv::CardType, fields);
}

template<> void initType<sv::Card>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::Card, type),
	};
	sf_struct(t, sv::Card, fields);

	t->postSerializeFn = [](void *inst, sf::Type *) {
		((sv::Card*)inst)->refresh();
	};
}

template<> void initType<sv::MapChunk>(Type *t)
{
	static Field fields[] = {
		sf_field_flags(sv::MapChunk, tiles, Field::CompactString),
		sf_field(sv::MapChunk, numNonZeroTiles),
	};
	sf_struct(t, sv::MapChunk, fields, Type::IsPod);
}

template<> void initType<sv::Entity::Type>(Type *t)
{
	static EnumValue values[] = {
		sf_enum(sv::Entity, None),
		sf_enum(sv::Entity, Character),
	};
	sf_enum_type(t, sv::Entity::Type, values);
}

template<> void initType<sv::Entity>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::Entity, id),
		sf_field(sv::Entity, position),
		sf_field(sv::Entity, blocksTile),
	};
	static PolymorphType polys[] = {
		sf_poly(sv::Entity, Character, sv::Character),
	};
	sf_struct_poly(t, sv::Entity, type, fields, polys);
}

template<> void initType<sv::Character>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::Character, name),
		sf_field(sv::Character, model),
		sf_field(sv::Character, players),
		sf_field(sv::Character, cards),
	};
	sf_struct_base(t, sv::Character, sv::Entity, fields);
}

template<> void initType<sv::Map>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::Map, tileTypes),
		sf_field(sv::Map, chunks),
	};
	sf_struct(t, sv::Map, fields);
}

template<> void initType<sv::State>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::State, map),
		sf_field(sv::State, entities),
	};
	sf_struct(t, sv::State, fields);
}


}
