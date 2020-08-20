#pragma once

#include "server/ServerState.h"

#include "client/System.h"

namespace cl {

struct BlobShadowSystem : EntitySystem
{
	static sf::Box<BlobShadowSystem> create();

	virtual void addBlobShadow(Systems &systems, uint32_t entityId, uint8_t componentIndex, const sv::BlobShadowComponent &c, const Transform &transform) = 0;

	virtual void updatePositions(AreaSystem *areaSystem, const BoneUpdates &boneUpdates, const FrameArgs &frameArgs) = 0;

	virtual void renderMain(const VisibleAreas &visibleAreas, const RenderArgs &renderArgs) = 0;
};

}
