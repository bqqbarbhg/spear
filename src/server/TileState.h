#pragma once

#include "server/ServerState.h"
#include "sf/HashMap.h"
#include "sf/HashSet.h"
#include "sf/Vector.h"

namespace sv {

struct TileChunk
{
	sf::HashSet<EntityId> entities;
};

struct EntityChunks
{
	sf::SmallArray<sf::Vec2i, 1> chunks;
};

struct TileState
{
	const State *state;

	sf::HashMap<sf::Vec2i, TileChunk> tileChunks;
	sf::HashMap<EntityId, EntityChunks> entityChunks;

	void update(const State *state, const DirtyList &dirty);
};

}
