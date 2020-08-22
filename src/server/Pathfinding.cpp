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

}

