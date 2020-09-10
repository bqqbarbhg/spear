#pragma once

#include "server/ServerState.h"
#include "server/Pathfinding.h"
#include "sf/Vector.h"
#include "sf/Random.h"

namespace sv {

struct EnemyState
{
	uint32_t id;
	uint32_t targetId = 0;
};

struct AiState
{
	ReachableSet reachableSet;
	sf::Random rng;
	sf::ImplicitHashMap<EnemyState, KeyId> enemies;
};

bool doEnemyActions(AiState &ai, sf::Array<sf::Box<sv::Event>> &events, sv::ServerState &state);

}
