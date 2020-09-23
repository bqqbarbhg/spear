#pragma once

#include "server/ServerState.h"

#include "client/System.h"

namespace sf { struct FastRay; }

namespace cl {

struct ModelSystem : EntitySystem
{
	static sf::Box<ModelSystem> create();

	virtual sf::Box<void> preloadModel(const sv::DynamicModelComponent &c) = 0;

	virtual void addModel(Systems &systems, uint32_t entityId, uint8_t componentIndex, const sv::DynamicModelComponent &c, const Transform &transform) = 0;
	virtual void updateLoadQueue(AreaSystem *areaSystem) = 0;

	virtual void renderMain(const LightSystem *lightSystem, const EnvLightSystem *envLightSystem, VisFogSystem *visFogSystem,const VisibleAreas &visibleAreas, const RenderArgs &renderArgs) = 0;

	virtual void editorPick(sf::Array<EntityHit> &hits, const sf::FastRay &ray, uint32_t userId) const = 0;

};

}
