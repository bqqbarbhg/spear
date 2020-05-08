#include "GameState.h"

#include "sf/Reflection.h"

namespace sv {

sf_inline void resolveChunk(sf::Vec2i &chunkI, sf::Vec2i &tileI, const sf::Vec2i &pos)
{
	chunkI = { pos.x >> MapChunk::SizeLog2, pos.y >> MapChunk::SizeLog2 };
	tileI = { pos.x & (MapChunk::Size - 1), pos.y & (MapChunk::Size - 1) };
}

void Map::setTile(const sf::Vec2i &pos, TileId tileId)
{
	sf::Vec2i chunkI, tileI;
	resolveChunk(chunkI, tileI, pos);
	auto result = chunks.insert(chunkI);
	MapChunk &chunk = result.entry.val;
	TileId &dst = chunks[chunkI].tiles[tileI.y * MapChunk::Size + tileI.x];
	if (dst == 0 && tileId != 0) chunk.numNonZeroTiles++;
	if (dst != 0 && tileId == 0) chunk.numNonZeroTiles--;
	dst = tileId;
	if (chunk.numNonZeroTiles == 0) {
		chunks.removeAt(&result.entry);
	}
}

TileId Map::getTile(const sf::Vec2i &pos) const
{
	sf::Vec2i chunkI, tileI;
	resolveChunk(chunkI, tileI, pos);
	auto it = chunks.find(chunkI);
	if (!it) return 0;
	return it->val.tiles[tileI.y * MapChunk::Size + tileI.x];
}

}

namespace sf {

template<> void initType<sv::TileType>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::TileType, name),
		sf_field(sv::TileType, floor),
		sf_field(sv::TileType, wall),
	};
	sf_struct(t, sv::TileType, fields);
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
		sf_field(sv::Entity, position),
		sf_field(sv::Entity, type),
		sf_field(sv::Entity, index),
	};
	sf_struct(t, sv::Entity, fields, Type::IsPod);
}

template<> void initType<sv::Character>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::Character, name),
	};
	sf_struct(t, sv::Character, fields);
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
		sf_field(sv::State, characters),
	};
	sf_struct(t, sv::State, fields);
}


}
