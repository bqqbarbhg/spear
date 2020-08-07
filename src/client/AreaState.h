#pragma once

#include "server/ServerState.h"
#include "client/EntityState.h"
#include "sf/Geometry.h"
#include "ext/sokol/sokol_defs.h"
#include "sf/Geometry.h"

namespace sf {
	struct Frustum;
}

namespace cl {

struct BoxArea
{
	sf::Bounds3 bounds;
	uint32_t areaId;
	uint32_t groupId;
	uint32_t userId;
	uint32_t entityId;
};

struct SphereArea
{
	sf::Sphere bounds;
	uint32_t areaId;
	uint32_t groupId;
	uint32_t userId;
	uint32_t entityId;
};

struct SpatialNode
{
	sf::Cube3 bounds;
	sf::Array<BoxArea> boxes;
	sf::Array<SphereArea> spheres;
};

struct AreaIds
{
	uint32_t areaId;
	uint32_t groupId;
	uint32_t userId;
	uint32_t entityId;
};

struct AreaGroupState
{
	sf::Array<AreaIds> newVisible;
	sf::Array<AreaIds> newInvisible;
};

enum AreaGroup {
	AreaVisibility,
	AreaPointLight,
	AreaGroup_Count,
};

struct AreaState
{
	static sf::Box<AreaState> create();

	uint32_t addGroup();
	void removeGroup(uint32_t groupId);

	uint32_t addEntityBox(uint32_t groupId, uint32_t userId, uint32_t entityId, const sf::Bounds3 &localBounds);
	void updateEntityBox(uint32_t areaId, const sf::Bounds3 &localBounds);
	void removeEntityBox(uint32_t areaId);

	uint32_t addWorldBox(uint32_t groupId, uint32_t userId, uint32_t entityId, const sf::Bounds3 &bounds);
	void updateWorldBox(uint32_t areaId, const sf::Bounds3 &bounds);
	void removeWorldBox(uint32_t areaId);

	void updateAnyBox(uint32_t areaId, const sf::Bounds3 &localBounds);
	void removeAnyBox(uint32_t areaId);

	uint32_t addEntitySphere(uint32_t groupId, uint32_t userId, uint32_t entityId, const sf::Sphere &localBounds);
	void updateEntitySphere(uint32_t areaId, const sf::Sphere &localBounds);
	void removeEntitySphere(uint32_t areaId);

	uint32_t addWorldSphere(uint32_t groupId, uint32_t userId, uint32_t entityId, const sf::Sphere &bounds);
	void updateWorldSphere(uint32_t areaId, const sf::Sphere &bounds);
	void removeWorldSphere(uint32_t areaId);

	void updateAnySphere(uint32_t areaId, const sf::Sphere &localBounds);
	void removeAnySphere(uint32_t areaId);

	bool isAreaVisible(uint32_t areaId) const;

	uint32_t findArea(uint32_t groupId, uint32_t userId) const;

	sf_forceinline void updateEntityBox(uint32_t groupId, uint32_t userId, const sf::Bounds3 &bounds) { updateEntityBox(findArea(groupId, userId), bounds); }
	sf_forceinline void removeEntityBox(uint32_t groupId, uint32_t userId) { removeEntityBox(findArea(groupId, userId)); }
	sf_forceinline void updateWorldBox(uint32_t groupId, uint32_t userId, const sf::Bounds3 &bounds) { updateWorldBox(findArea(groupId, userId), bounds); }
	sf_forceinline void removeWorldBox(uint32_t groupId, uint32_t userId) { removeWorldBox(findArea(groupId, userId)); }
	sf_forceinline void updateAnyBox(uint32_t groupId, uint32_t userId, const sf::Bounds3 &bounds) { updateAnyBox(findArea(groupId, userId), bounds); }
	sf_forceinline void removeAnyBox(uint32_t groupId, uint32_t userId) { removeAnyBox(findArea(groupId, userId)); }
	sf_forceinline void updateEntitySphere(uint32_t groupId, uint32_t userId, const sf::Sphere &bounds) { updateEntitySphere(findArea(groupId, userId), bounds); }
	sf_forceinline void removeEntitySphere(uint32_t groupId, uint32_t userId) { removeEntitySphere(findArea(groupId, userId)); }
	sf_forceinline void updateWorldSphere(uint32_t groupId, uint32_t userId, const sf::Sphere &bounds) { updateWorldSphere(findArea(groupId, userId), bounds); }
	sf_forceinline void removeWorldSphere(uint32_t groupId, uint32_t userId) { removeWorldSphere(findArea(groupId, userId)); }
	sf_forceinline void updateAnySphere(uint32_t groupId, uint32_t userId, const sf::Sphere &bounds) { updateAnySphere(findArea(groupId, userId), bounds); }
	sf_forceinline void removeAnySphere(uint32_t groupId, uint32_t userId) { removeAnySphere(findArea(groupId, userId)); }

	void updateEntityTransform(uint32_t entityId, const sf::Mat34 &transform);

	void querySpatialNodesFrustum(sf::Array<const SpatialNode*> &nodes, const sf::Frustum &frustum) const;
	const AreaGroupState &getGroupState(uint32_t groupId) const;

	void updateMainVisibility(sf::Slice<const SpatialNode*> nodes, const sf::Frustum &frustum);
};

}
