#pragma once

#include "server/ServerState.h"

#include "client/System.h"

namespace sf { struct FastRay; }

namespace cl {

struct TileModelSystem : EntitySystem
{
	static sf::Box<TileModelSystem> create();

	virtual void addModel(Systems &systems, uint32_t entityId, uint8_t componentIndex, const sv::TileModelComponent &c, const Transform &transform) = 0;

	virtual void garbageCollectChunks(AreaSystem *areaSystem, const FrameArgs &frameArgs) = 0;
	virtual void updateLoadQueue(AreaSystem *areaSystem) = 0;

	virtual void uploadVisibleChunks(const VisibleAreas &activeAreas, AreaSystem *areaSystem, const FrameArgs &frameArgs) = 0;

	virtual void renderShadow(const VisibleAreas &shadowAreas, const RenderArgs &renderArgs) = 0;
	virtual void renderMain(const LightSystem *lightSystem, const EnvLightSystem *envLightSystem, const VisibleAreas &visibleAreas, const RenderArgs &renderArgs) = 0;
	virtual void renderEnvmapGBuffer(const VisibleAreas &envmapAreas, const RenderArgs &renderArgs) = 0;

	virtual void editorPick(sf::Array<EntityHit> &hits, const sf::FastRay &ray, uint32_t userId) const = 0;
};

}
