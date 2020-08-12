#pragma once

#include "server/ServerState.h"

#include "client/System.h"

namespace cl {

struct ParticleSystem : EntitySystem
{
	static sf::Box<ParticleSystem> create(const SystemsDesc &desc);

	virtual void addEffect(Systems &systems, uint32_t entityId, uint8_t componentIndex, const sf::Box<sv::ParticleSystemComponent> &c, const Transform &transform) = 0;

	virtual void updateParticles(const VisibleAreas &visibleAreas, float dt) = 0;

	virtual void renderMain(const VisibleAreas &visibleAreas, const RenderArgs &renderArgs) = 0;

};

}
