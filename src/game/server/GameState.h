#pragma once

#include "sf/Array.h"
#include "sf/HashMap.h"
#include "sf/Vector.h"
#include "sf/Symbol.h"
#include "sf/Box.h"

namespace sv {

typedef uint16_t TileId;
typedef uint32_t EntityId;
struct Event;
struct Action;

struct TileType
{
	sf::Symbol name;
	bool floor = false;
	bool wall = false;
};

struct MapChunk
{
	static constexpr const uint32_t SizeLog2 = 4;
	static constexpr const uint32_t Size = 1u << SizeLog2;
	TileId tiles[Size * Size] = { };
	uint32_t numNonZeroTiles = 0;
};

struct Entity
{
	#if SF_DEBUG
		virtual void debugForceVtable() { }
	#endif

	enum Type
	{
		None,
		Character,

		Type_Count,
		Type_Force32 = 0x7fffffff,
	};

	EntityId id = 0;
	sf::Vec2i position;
	Type type;
	bool blocksTile = false;

	Entity() { }
	Entity(Type type) : type(type) { }

	template <typename T> T *as() { return type == T::EntityType ? (T*)this : nullptr; }
	template <typename T> const T *as() const { return type == T::EntityType ? (T*)this : nullptr; }
};

template <Entity::Type SelfType>
struct EntityBase : Entity
{
	static constexpr Type EntityType = SelfType;
	EntityBase() : Entity(SelfType) { }
};

struct Character : EntityBase<Entity::Character>
{
	sf::Symbol name;
	sf::Array<uint32_t> players;

	Character()
	{
		blocksTile = true;
	}
};

struct Map
{
	sf::Array<TileType> tileTypes;
	sf::HashMap<sf::Vec2i, MapChunk> chunks;

	void setTile(const sf::Vec2i &pos, TileId tileId);
	TileId getTile(const sf::Vec2i &pos) const;

	bool canStandOn(const sf::Vec2i &pos) const;
};

struct EntityTileMap
{
	rhmap map;

	EntityTileMap();
	EntityTileMap(const EntityTileMap &rhs);
	EntityTileMap(EntityTileMap &&rhs);
	EntityTileMap &operator=(const EntityTileMap &rhs);
	EntityTileMap &operator=(EntityTileMap &&rhs);
	~EntityTileMap();

	void add(EntityId entity, const sf::Vec2i &newPos);
	void update(EntityId entity, const sf::Vec2i &oldPos, const sf::Vec2i &newPos);
	void remove(EntityId entity, const sf::Vec2i &oldPos);
	void reserve(size_t size);
	void clear();

	void impGrow(size_t minSize);
};

struct State
{
	Map map;
	sf::Array<sf::Box<Entity>> entities;
	EntityTileMap entityTileMap;

	void refreshEntityTileMap();

	void initEntity(EntityId entity, sf::Box<Entity> data);
	void destroyEntity(EntityId entity);
	void setEntityPosition(EntityId entity, const sf::Vec2i &pos);

	void getEntitiesOnTile(sf::Array<Entity*> &dst, const sf::Vec2i &pos) const;

	bool canStandOn(const sf::Vec2i &pos) const;

	void applyEvent(Event *event);
	bool applyAction(Action *action, sf::Array<sf::Box<Event>> &events, sf::StringBuf &error);
};

}
