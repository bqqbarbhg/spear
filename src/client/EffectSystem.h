#pragma once

#include "server/ServerState.h"

#include "client/System.h"

namespace sf { struct FastRay; }

namespace cl {

struct EffectSystem : EntitySystem
{
	static sf::Box<EffectSystem> create();

	virtual uint32_t spawnOneShotEffect(Systems &systems, const sf::Symbol &prefabName, const sf::Vec3 &position) = 0;

	virtual uint32_t spawnAttachedEffect(Systems &systems, const sf::Symbol &prefabName, uint32_t parentId, const sf::Vec3 &offset) = 0;
	virtual void removeAttachedEffect(uint32_t effectId) = 0;

	virtual void update(Entities &entities, const FrameArgs &frameArgs) = 0;

};

}
