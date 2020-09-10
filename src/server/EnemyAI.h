#pragma once

#include "server/ServerState.h"
#include "server/Pathfinding.h"
#include "sf/Vector.h"
#include "sf/Random.h"

namespace sv {

struct AiState
{
	ReachableSet reachableSet;
	sf::Random rng;
};

bool getEnemyActions(AiState &ai, sf::Array<sf::Box<sv::Action>> &actions, const sv::ServerState &state);

}
