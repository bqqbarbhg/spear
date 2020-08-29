#include "Pathfinding.h"

#include "sf/Heap.h"

#include "server/ServerState.h"

namespace sv {

struct ReachableQueueItem
{
	sf::Vec2i tile;
	uint32_t distance;

	bool operator<(const ReachableQueueItem &rhs) const {
		return distance < rhs.distance;
	}
};

static const sf::Vec2i cardinalDirections[] = {
	{ 1, 0 }, { 0, -1 }, { -1, 0 }, { 0, 1 },
};

bool isBlockedByPropOrCharacter(void *user, const ServerState &state, const sf::Vec2i &tile)
{
	uint32_t id;
	sf::UintFind find = state.getTileEntities(tile);
	while (find.next(id)) {
		IdType type = getIdType(id);
		if (type == IdType::Prop || type == IdType::Character) return true;
	}
	return false;
}

bool isBlockedByPropOrRoomConnection(void *user, const ServerState &state, const sf::Vec2i &tile)
{
	uint32_t id;
	sf::UintFind find = state.getTileEntities(tile);
	while (find.next(id)) {
		IdType type = getIdType(id);
		if (type == IdType::Prop || type == IdType::RoomConnection) return true;
	}
	return false;
}

void findReachableSet(ReachableSet &set, const ServerState &state, const PathfindOpts &opts, const sf::Vec2i &tile)
{
	set.distanceToTile.clear();

	sf::SmallArray<ReachableQueueItem, 256> queue;

	if (opts.maxDistance > 0) {
		queue.push({ tile, 0 });
	}

	while (queue.size > 0) {
		ReachableQueueItem item = sf::priorityDequeue(queue);

		for (const sf::Vec2i &dir : cardinalDirections) {
			sf::Vec2i nbTile = item.tile + dir;
			uint32_t nbDist = item.distance + 1;

			if (nbTile == tile || opts.isBlockedFn(opts.isBlockedUser, state, nbTile)) {
				continue;
			}

			auto res = set.distanceToTile.insert(nbTile);
			if (res.inserted || nbDist < res.entry.val.distance) {
				res.entry.val.previous = item.tile;
				res.entry.val.distance = nbDist;
				if (nbDist + 1 <= opts.maxDistance) {
					sf::priorityEnqueue(queue, ReachableQueueItem{ nbTile, nbDist });
				}
			}
		}
	}
}

void findRoomArea(RoomTiles &roomTiles, const ServerState &state, uint32_t maxRadius)
{
	roomTiles.clear();

	sf::UintSet visited;
	sf::UintSet inside;

	static const sf::Vec2i dirs[] = {
		{ -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 },
	};

	ReachableSet reachable;

	for (sf::UintKeyVal keyVal : state.entityToTile) {
		sf::Vec2i tile = unpackTile(keyVal.val);
		if (!isBlockedByPropOrRoomConnection(nullptr, state, tile)) continue;

		if (visited.insertIfNew(keyVal.val)) {
			inside.insertDuplicate(keyVal.val);
		}

		for (const sf::Vec2i &dir : dirs) {
			sf::Vec2i nb = tile + dir;
			uint32_t packedNb = packTile(nb);
			if (visited.findOne(packedNb)) continue;
			if (isBlockedByPropOrRoomConnection(nullptr, state, nb)) {
				continue;
			}

			PathfindOpts opts;
			opts.maxDistance = maxRadius;
			opts.isBlockedFn = &isBlockedByPropOrRoomConnection;
			findReachableSet(reachable, state, opts, nb);

			bool isInside = true;
			for (auto &pair : reachable.distanceToTile) {
				if (pair.val.distance == maxRadius) {
					isInside = false;
				}
			}

			visited.insertDuplicate(packedNb);
			if (isInside) {
				inside.insertDuplicate(packedNb);
			}

			for (auto &pair : reachable.distanceToTile) {
				uint32_t packed = packTile(pair.key);
				if (visited.insertIfNew(packed)) {
					if (isInside) {
						inside.insertDuplicate(packed);
					}
				}
			}
		}
	}

	for (uint32_t packedTile : inside) {
		sf::Vec2i tile = unpackTile(packedTile);
		bool border = false;
		for (const sf::Vec2i &dir : dirs) {
			sf::Vec2i nb = tile + dir;
			uint32_t packedNb = packTile(nb);
			if (!inside.findOne(packedNb)) {
				border = true;
				break;
			}
		}

		if (border) {
			roomTiles.border.insert(tile);
		} else {
			roomTiles.interior.insert(tile);
		}
	}
}

}

