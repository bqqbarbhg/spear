#pragma once

#include "sf/Array.h"
#include "sf/Vector.h"
#include "sf/HashMap.h"

namespace sv {

struct ServerState;
struct RoomTiles;

typedef bool IsBlockedFn(void *user, const ServerState &state, const sf::Vec2i &tile);

struct PathfindOpts
{
	IsBlockedFn *isBlockedFn = nullptr;
	void *isBlockedUser = nullptr;

	uint32_t maxDistance;
};

struct ReachableTile
{
	sf::Vec2i previous;
	uint32_t distance = 0;
};

struct ReachableSet
{
	sf::HashMap<sf::Vec2i, ReachableTile> distanceToTile;
};

bool isBlockedByPropOrCharacter(void *user, const ServerState &state, const sf::Vec2i &tile);
bool isBlockedByPropOrRoomConnection(void *user, const ServerState &state, const sf::Vec2i &tile);

void findReachableSet(ReachableSet &set, const ServerState &state, const PathfindOpts &opts, const sf::Vec2i &tile);

void findRoomArea(RoomTiles &roomTiles, const ServerState &state, uint32_t maxRadius=64);

}
