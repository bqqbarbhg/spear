#include "EnemyAI.h"

#include "server/LineRasterizer.h"

namespace sv {

struct CardToUse
{
	const sv::Prefab *prefab;
	uint32_t cardId = 0;
	float weight = 0.0f;
};

template <typename T>
static void shuffle(sf::Slice<T> ts, sf::Random &rng)
{
	for (size_t i = ts.size; i > 1; i--) {
		size_t j = rng.nextU32() % i;
		if (i != j) {
			sf::impSwap(ts[i - 1], ts[j]);
		}
	}
}

bool tryUseCard(AiState &ai, sf::Array<sf::Box<sv::Event>> &events, sv::ServerState &state, CardToUse &cardToUse)
{
	uint32_t selfId = state.turnInfo.characterId;
	const Character *self = state.characters.find(selfId);
	if (!self) return false;

	for (uint32_t targetId : ai.reachTargets)
	for (const sf::Vec2i &tile : ai.reachTiles)
	{
		if (state.canTarget(selfId, targetId, cardToUse.prefab->name, tile)) {
			if (tile != self->tile) {
				ReachableTile *reach = ai.reachableSet.distanceToTile.findValue(tile);
				if (!reach) continue;

				MoveAction action = { };
				action.characterId = selfId;
				action.tile = tile;
				action.waypoints.resizeUninit(reach->distance);
				uint32_t waypointIx = action.waypoints.size;
				action.waypoints[--waypointIx] = tile;
				sf::Vec2i prev = reach->previous;
				while (sv::ReachableTile *prevReach = ai.reachableSet.distanceToTile.findValue(prev)) {
					action.waypoints[--waypointIx] = prev;
					prev = prevReach->previous;
				}
				if (!state.requestAction(events, action)) continue;
			}

			UseCardAction action = { };
			action.characterId = selfId;
			action.targetId = targetId;
			action.cardId = cardToUse.cardId;
			if (state.requestAction(events, action)) {
				return true;
			}
		}
	}

	return false;
}

bool updateTargets(AiState &ai, uint32_t characterId, sv::ServerState &state)
{
	const Character *chr = state.characters.find(characterId);
	if (!chr) return false;

	const int32_t targetRange = 20;
	const uint32_t targetRememberTurns = 4;

	EnemyState &enemyState = ai.enemies[characterId];
	enemyState.id = characterId;

	// Find new targets
	// TODO: Filter for performance?
	for (Character &maybeTargetChr : state.characters) {
		if (maybeTargetChr.enemy) continue;

		sf::Vec2i delta = maybeTargetChr.tile - chr->tile;
		if (delta.x < 0) delta.x = -delta.x;
		if (delta.y < 0) delta.y = -delta.y;
		int32_t dist = delta.x + delta.y;
		if (dist > targetRange) continue;

		bool hitProp = false;
		bool hitWall = false;

		sv::ConservativeLineRasterizer raster(chr->tile, maybeTargetChr.tile);
		for (;;) {
			sf::Vec2i tile = raster.next();
			if (tile == maybeTargetChr.tile) break;
			if (tile == chr->tile) continue;

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

		if (!hitWall) {
			EnemyTarget &target = enemyState.targets[maybeTargetChr.id];
			target.id = maybeTargetChr.id;
			target.turnsLeft = targetRememberTurns;
		}
	}

	return enemyState.targets.size() > 0;
}

bool doEnemyActions(AiState &ai, sf::Array<sf::Box<sv::Event>> &events, sv::ServerState &state)
{
	uint32_t chrId = state.turnInfo.characterId;
	const Character *chr = state.characters.find(chrId);
	if (!chr) return false;
	if (!chr->enemy) return false;

	if (!updateTargets(ai, chrId, state)) return false;

	EnemyState &enemyState = ai.enemies[chrId];
	enemyState.id = chrId;

	sf::SmallArray<CardToUse, 16> cardsToUse;
	float totalWeight = 0.0f;

	// Age targets
	for (uint32_t i = 0; i < enemyState.targets.size(); i++) {
		EnemyTarget &target = enemyState.targets.data[i];
		Character *targetChr = state.characters.find(target.id);

		if (--target.turnsLeft == 0 || !targetChr || targetChr->enemy) {
			enemyState.targets.remove(target.id);
			i--;
		}
	}

	// Add reachable targets
	ai.reachTargets.clear();
	ai.reachTargets.push(chrId);
	for (EnemyTarget &target : enemyState.targets) {
		ai.reachTargets.push(target.id);
	}
	shuffle(ai.reachTargets.slice(), ai.rng);

	for (uint32_t cardId : chr->selectedCards) {
		const Card *card = state.cards.find(cardId);
		if (!card || card->cooldownLeft > 0) continue;
		const Prefab *cardPrefab = state.prefabs.find(card->prefabName);
		if (!cardPrefab) continue;
		const CardComponent *cardComp = cardPrefab->findComponent<CardComponent>();
		if (!cardComp) continue;

		float weight = 1.0f;
		weight += (float)cardComp->cooldown * 0.5f;

		// Bias self healing items depending on HP left
		if (cardComp->targetSelf) {
			auto *castComp = cardPrefab->findComponent<CardCastComponent>();
			auto *spellPrefab = castComp ? state.prefabs.find(castComp->spellName) : NULL;
			auto *healComp = spellPrefab ? spellPrefab->findComponent<SpellHealComponent>() : NULL;
			if (healComp) {
				float healthLeft = sf::clamp((float)chr->health / (float)chr->maxHealth, 0.0f, 1.0f);
				weight *= sf::max(0.8f - healthLeft, 0.0f) * 10.0f;
			}
		}

		weight *= cardComp->aiWeight;

		cardsToUse.push({ cardPrefab, cardId, weight });
		totalWeight += weight;
	}

	sv::PathfindOpts opts;
	opts.isBlockedFn = &sv::isBlockedByPropOrCharacter;
	opts.maxDistance = state.turnInfo.movementLeft;
	sv::findReachableSet(ai.reachableSet, state, opts, chr->tile);

	ai.reachTiles.clear();
	ai.reachTiles.push(chr->tile);
	for (const auto &pair : ai.reachableSet.distanceToTile) {
		ai.reachTiles.push(pair.key);
	}
	shuffle(ai.reachTiles.slice(), ai.rng);

	while (cardsToUse.size > 0) {
		float w = ai.rng.nextFloat() * totalWeight;
		uint32_t chosenIx;
		for (chosenIx = 0; chosenIx < cardsToUse.size - 1; chosenIx++) {
			w -= cardsToUse[chosenIx].weight;
			if (w <= 0.0f) break;
		}

		if (tryUseCard(ai, events, state, cardsToUse[chosenIx])) return true;

		totalWeight -= cardsToUse[chosenIx].weight;
		cardsToUse.removeSwap(chosenIx);
	}

	// Could not do anything, maybe we can move closer to an enemy?

	if (enemyState.preferredTargetId) {
		if (!state.characters.find(enemyState.preferredTargetId)) {
			enemyState.preferredTargetId = 0;
		} else if (!enemyState.targets.find(enemyState.preferredTargetId)) {
			enemyState.preferredTargetId = 0;
		}
	}

	const uint32_t targetableDistance = sf::max(20u, state.turnInfo.movementLeft * 3);

	if (!enemyState.preferredTargetId) {
		sv::PathfindOpts opts;
		opts.isBlockedFn = &sv::isBlockedByProp;
		opts.maxDistance = targetableDistance;
		sv::findReachableSet(ai.targetableSet, state, opts, chr->tile);

		uint32_t distance = UINT32_MAX;
		for (uint32_t targetId : ai.reachTargets) {
			Character *targetChr = state.characters.find(targetId);
			if (!targetChr) continue;
			if (const ReachableTile *reach = ai.targetableSet.distanceToTile.findValue(targetChr->tile)) {
				uint32_t extraDist = ai.rng.nextU32() % 3;
				if (reach->distance + extraDist < distance) {
					distance = reach->distance;
					enemyState.preferredTargetId = targetId;
				}
			}
		}
	}

	if (Character *targetChr = state.characters.find(enemyState.preferredTargetId)) {
		sv::PathfindOpts opts;
		opts.isBlockedFn = &sv::isBlockedByProp;
		opts.maxDistance = targetableDistance;
		sv::findReachableSet(ai.targetableSet, state, opts, targetChr->tile);

		sf::KeyVal<sf::Vec2i, ReachableTile> *bestReach = nullptr;
		uint32_t bestDistance = UINT32_MAX;
		for (auto &reachPair : ai.reachableSet.distanceToTile) {
			if (ReachableTile *targetableReach = ai.targetableSet.distanceToTile.findValue(reachPair.key)) {
				if (targetableReach->distance < bestDistance) {
					bestReach = &reachPair;
					bestDistance = targetableReach->distance;
				}
			}
		}

		if (bestReach) {
			sf::Vec2i tile = bestReach->key;
			ReachableTile *reach = &bestReach->val;
			sf_assert(reach);

			MoveAction action = { };
			action.characterId = chrId;
			action.tile = tile;
			action.waypoints.resizeUninit(reach->distance);
			uint32_t waypointIx = action.waypoints.size;
			action.waypoints[--waypointIx] = tile;
			sf::Vec2i prev = reach->previous;
			while (sv::ReachableTile *prevReach = ai.reachableSet.distanceToTile.findValue(prev)) {
				action.waypoints[--waypointIx] = prev;
				prev = prevReach->previous;
			}
			if (state.requestAction(events, action)) return true;
		}
	}

	return false;
}

}
