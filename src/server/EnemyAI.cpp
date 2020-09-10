#include "EnemyAI.h"

namespace sv {

bool getEnemyActions(AiState &ai, sf::Array<sf::Box<sv::Action>> &actions, const sv::ServerState &state)
{
	uint32_t chrId = state.turnInfo.characterId;
	const Character *chr = state.characters.find(chrId);
	if (!chr) return false;
	if (!chr->enemy) return false;

	sv::PathfindOpts opts;
	opts.isBlockedFn = &sv::isBlockedByPropOrCharacter;
	opts.maxDistance = state.turnInfo.movementLeft;
	sv::findReachableSet(ai.reachableSet, state, opts, chr->tile);

	if (ai.reachableSet.distanceToTile.size() > 0) {
		uint32_t numTiles = ai.reachableSet.distanceToTile.size();
		uint32_t tileIx = ai.rng.nextU32() % numTiles;
		auto &pair = ai.reachableSet.distanceToTile.data[tileIx];

		auto action = sf::box<sv::MoveAction>();

		action->characterId = chrId;
		action->tile = pair.key;
		action->waypoints.resizeUninit(pair.val.distance);
		uint32_t waypointIx = action->waypoints.size;
		action->waypoints[--waypointIx] = pair.key;

		sf::Vec2i prev = pair.val.previous;
		while (sv::ReachableTile *prevReach = ai.reachableSet.distanceToTile.findValue(prev)) {
			action->waypoints[--waypointIx] = prev;
			prev = prevReach->previous;
		}

		actions.push(action);
	}

	return true;
}

}
