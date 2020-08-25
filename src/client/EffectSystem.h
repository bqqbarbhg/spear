#pragma once

#include "server/ServerState.h"

#include "client/System.h"

namespace sf { struct FastRay; }

namespace cl {

struct EffectSystem : EntitySystem
{
	static sf::Box<EffectSystem> create();

	virtual void spawnOneShotEffect(Systems &systems, const sf::Symbol &prefabName, const sf::Vec3 &position) = 0;

	virtual void update(Entities &entities, const FrameArgs &frameArgs) = 0;

};

}
