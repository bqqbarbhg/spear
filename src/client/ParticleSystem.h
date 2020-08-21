#pragma once

#include "server/ServerState.h"

#include "client/System.h"

namespace cl {

struct ParticleSystem : EntitySystem
{
	static sf::Box<ParticleSystem> create(const SystemsDesc &desc);

	virtual uint32_t reserveEffectType(Systems &systems, const sf::Box<sv::ParticleSystemComponent> &c) = 0;
	virtual void releaseEffectType(Systems &systems, const sf::Box<sv::ParticleSystemComponent> &c) = 0;

	virtual void addEffect(Systems &systems, uint32_t entityId, uint8_t componentIndex, const sf::Box<sv::ParticleSystemComponent> &c, const Transform &transform) = 0;

	virtual void updateParticles(const VisibleAreas &visibleAreas, const FrameArgs &frameArgs) = 0;

	virtual void renderMain(const VisibleAreas &visibleAreas, const RenderArgs &renderArgs) = 0;

};

}
