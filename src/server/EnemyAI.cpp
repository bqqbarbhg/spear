#include "EnemyAI.h"

#include "server/LineRasterizer.h"

namespace sv {

static double scorePosition(AiState &ai, const sv::ServerState &state, const Character *chr, const Character *targetChr, const sf::Vec2i &position)
{
	bool hitProp = false;
	bool hitWall = false;
	sf::Vec2i delta = position - targetChr->tile;
	if (delta.x < -0) delta.x = -delta.x;
	if (delta.y < -0) delta.y = -delta.y;
	uint32_t distance = delta.x + delta.y;

	sv::ConservativeLineRasterizer raster(position, targetChr->tile);
	for (;;) {
		sf::Vec2i tile = raster.next();
		if (tile == targetChr->tile) break;
		if (tile == position) continue;


		uint32_t id;
		sf::UintFind find = state.getTileEntities(tile);
		while (find.next(id)) {
			IdType type = getIdType(id);
			if (type == IdType::Prop) {
				hitProp = true;
				const Prop *prop = state.props.find(id);
				if (prop && (prop->flags & Prop::Wall) != 0) hitWall = true;
			}
		}
	}

	double score = 1000.0;

	score -= (double)distance;
	if (hitWall) score -= 100.0;
	if (hitProp) score -= 5.0;

	return score;
}

bool doEnemyActions(AiState &ai, sf::Array<sf::Box<sv::Event>> &events, sv::ServerState &state)
{
	uint32_t chrId = state.turnInfo.characterId;
	const Character *chr = state.characters.find(chrId);
	if (!chr) return false;
	if (!chr->enemy) return false;

	EnemyState &enemyState = ai.enemies[chrId];
	enemyState.id = chrId;

	const Character *targetChr = state.characters.find(enemyState.targetId);
	if (!targetChr) {
		uint32_t bestTargetId = 0;
		double bestScore = 0.0;

		for (const Character &maybeTargetChr : state.characters) {
			if (maybeTargetChr.enemy) continue;

			bool hitProp = false;
			bool hitWall = false;
			uint32_t distance = 0;

			sv::ConservativeLineRasterizer raster(chr->tile, maybeTargetChr.tile);
			for (;;) {
				sf::Vec2i tile = raster.next();
				if (tile == maybeTargetChr.tile) break;
				if (tile == chr->tile) continue;

				distance++;

				uint32_t id;
				sf::UintFind find = state.getTileEntities(tile);
				while (find.next(id)) {
					IdType type = getIdType(id);
					if (type == IdType::Prop) {
						hitProp = true;
						const Prop *prop = state.props.find(id);
						if (prop && (prop->flags & Prop::Wall) != 0) hitWall = true;
					}
				}
			}

			double score = 12.0;

			if (hitWall) score -= 10.0;
			score -= (double)distance;
			if (hitProp) score *= 0.5;

			if (score > bestScore) {
				bestScore = score;
				bestTargetId = maybeTargetChr.id;
			}
		}

		enemyState.targetId = bestTargetId;
		targetChr = state.characters.find(enemyState.targetId);
	}

	if (!targetChr) return true;

	sv::PathfindOpts opts;
	opts.isBlockedFn = &sv::isBlockedByPropOrCharacter;
	opts.maxDistance = state.turnInfo.movementLeft;
	sv::findReachableSet(ai.reachableSet, state, opts, chr->tile);

	if (ai.reachableSet.distanceToTile.size() > 0) {

		sf::Vec2i bestTile = chr->tile;
		double bestScore = scorePosition(ai, state, chr, targetChr, chr->tile);

		for (auto &pair : ai.reachableSet.distanceToTile) {
			double score = scorePosition(ai, state, chr, targetChr, pair.key);

			if (score > bestScore) {
				bestScore = score;
				bestTile = pair.key;
			}
		}

		if (bestScore > 0.0) {
			ReachableTile *reach = ai.reachableSet.distanceToTile.findValue(bestTile);
			if (reach) {
				MoveAction action = { };
				action.characterId = chrId;
				action.tile = bestTile;
				action.waypoints.resizeUninit(reach->distance);
				uint32_t waypointIx = action.waypoints.size;
				action.waypoints[--waypointIx] = bestTile;

				sf::Vec2i prev = reach->previous;
				while (sv::ReachableTile *prevReach = ai.reachableSet.distanceToTile.findValue(prev)) {
					action.waypoints[--waypointIx] = prev;
					prev = prevReach->previous;
				}

				if (!state.requestAction(events, action)) return true;
			}
		}

		for (uint32_t cardId : chr->selectedCards) {
			const Card *card = state.cards.find(cardId);
			if (!card) continue;

			if (state.canTarget(chrId, targetChr->id, card->prefabName)) {
				UseCardAction action;
				action.characterId = chrId;
				action.cardId = cardId;
				action.targetId = targetChr->id;
				if (!state.requestAction(events, action)) return true;
				break;
			}
		}
	}

	return true;
}

}
