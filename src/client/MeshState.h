#pragma once

#include "sf/Box.h"
#include "sf/Matrix.h"
#include "server/ServerState.h"

namespace cl {

struct RenderArgs;
struct SpatialNode;

struct MeshState
{
	static sf::Box<MeshState> create();

	void addMesh(uint32_t entityId, const sf::Box<sv::ModelComponent> &c);
	void updateEntityTransform(uint32_t entityId, const sf::Mat34 &transform);
	void removeEntity(uint32_t entityId);

	void updatePendingMeshes();

	void renderShadows(sf::Slice<const SpatialNode*> visibleNodes, const RenderArgs &args);
	void renderMeshses(const RenderArgs &args);
};

}