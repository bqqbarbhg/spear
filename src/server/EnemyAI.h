#pragma once

#include "server/ServerState.h"
#include "server/Pathfinding.h"
#include "sf/Vector.h"
#include "sf/Random.h"

namespace sv {

struct EnemyTarget
{
	uint32_t id;
	uint32_t turnsLeft;
};

struct EnemyState
{
	uint32_t id;
	sf::ImplicitHashMap<EnemyTarget, KeyId> targets;
	uint32_t preferredTargetId = 0;
};

struct AiState
{
	ReachableSet reachableSet;
	ReachableSet targetableSet;
	sf::Random rng;
	sf::ImplicitHashMap<EnemyState, KeyId> enemies;
	uint32_t selfId;
	sf::Array<sf::Vec2i> reachTiles;
	sf::Array<uint32_t> reachTargets;
};

bool updateTargets(AiState &ai, uint32_t characterId, sv::ServerState &state);
bool doEnemyActions(AiState &ai, sf::Array<sf::Box<sv::Event>> &events, sv::ServerState &state);

}
