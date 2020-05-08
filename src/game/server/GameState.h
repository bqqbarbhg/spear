#pragma once

#include "sf/Array.h"
#include "sf/HashMap.h"
#include "sf/Vector.h"
#include "sf/Symbol.h"

namespace sv {

typedef uint16_t TileId;
typedef uint32_t EntityId;

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
	enum Type
	{
		None,
		Character,

		Type_Count,
		Type_Force32 = 0x7fffffff,
	};

	sf::Vec2i position;
	Type type;
	uint32_t index;
};

struct Character
{
	sf::Symbol name;
};

struct Map
{
	rhmap entityTileMap;

	sf::Array<TileType> tileTypes;
	sf::HashMap<sf::Vec2i, MapChunk> chunks;

	void setTile(const sf::Vec2i &pos, TileId tileId);
	TileId getTile(const sf::Vec2i &pos) const;
};

struct State
{
	Map map;
	sf::Array<Entity> entities;
	sf::Array<Character> characters;
};

}
