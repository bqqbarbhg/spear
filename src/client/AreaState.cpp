#include "AreaState.h"

#include "sf/Frustum.h"
#include "sf/Unique.h"
#include "sf/UintMap.h"

#include "client/ClientGlobal.h"
#include "client/ClientState.h"

#include "sf/ext/mx/mx_platform.h"

namespace cl {

static const float SpatialNodeTopLevelSize = 128.0f;
static const float SpatialNodePaddingRatio = 0.25f;
static const uint32_t SpatialNodeMaxDepth = 4;

struct SpatialChildren;

struct SpatialNodeImp : SpatialNode
{
	float unpaddedExtent;
	float maxLeafExtent;
	float minLeafExtent;
	uint32_t depth;
	uint32_t childMask;
	sf::Unique<SpatialChildren> children;
};

struct SpatialChildren
{
	SpatialNodeImp child[8];
};

struct AreaImp
{
	SpatialNodeImp *spatialNode;
	uint32_t spatialIndex;
	uint32_t attachEntityId;
	uint32_t groupId;
	uint32_t userId;
	uint32_t entityId;
};

union AreaLocalBounds
{
	AreaLocalBounds() { }
	sf::Bounds3 box;
	sf::Sphere sphere;
};

struct GroupImp
{
	sf::UintMap areas;
	AreaGroupState state;
};

struct EntityAreasImp
{
	sf::SmallArray<uint32_t, 2> boxes;
	sf::SmallArray<uint32_t, 2> spheres;
};

struct AreaStateImp : AreaState
{
	sf::Array<GroupImp> groups;
	sf::Array<uint32_t> freeGroupIndices;

	sf::Array<AreaImp> areas;
	sf::Array<AreaLocalBounds> areaLocalBounds;
	sf::Array<uint32_t> freeAreaIndices;

	// TODO: Bitmap
	sf::Array<uint32_t> areaVisibility;
	sf::Array<uint32_t> prevAreaVisibility;

	sf::HashMap<sf::Vec3i, sf::Unique<SpatialNodeImp>> topSpatialNodes;
	sf::HashMap<uint32_t, EntityAreasImp> entityAreas;

	sf_forceinline uint32_t allocateAreaId()
	{
		if (freeAreaIndices.size > 0) {
			return freeAreaIndices.popValue();
		} else {
			if (areas.size % 32 == 0) {
				areaVisibility.push(0);
				prevAreaVisibility.push(0);
			}
			areas.push();
			areaLocalBounds.push();
			return areas.size - 1;
		}
	}
};

static void walkSpatialNode(sf::Array<const SpatialNode*> &nodes, const SpatialNodeImp *rootNode, const sf::Frustum &frustum)
{
	if (!frustum.intersects(rootNode->bounds)) return;

	nodes.push(rootNode);

	for (uint32_t nodeI = 0; nodeI < nodes.size; nodeI++) {
		const SpatialNodeImp *node = (const SpatialNodeImp*)nodes[nodeI];

		const SpatialNodeImp *children = node->children->child;
		uint32_t childMask = node->childMask;
		if (childMask) {
			for (uint32_t i = 0; i < 8; i++) {
				if ((childMask & (1 << i)) == 0) continue;

				const SpatialNodeImp *child = &children[i];
				if (frustum.intersects(child->bounds)) {
					nodes.push(child);
				}
			}
		}
	}
}

static sf_noinline sf::Unique<SpatialNodeImp> initializeSpatialRoot(const sf::Vec3i &key)
{
	sf::Unique<SpatialNodeImp> root = sf::unique<SpatialNodeImp>();
	root->unpaddedExtent = SpatialNodeTopLevelSize;
	root->maxLeafExtent = SpatialNodeTopLevelSize * SpatialNodePaddingRatio;
	root->childMask = 0;
	root->depth = 0;
	root->bounds.origin = sf::Vec3(key) * SpatialNodeTopLevelSize;
	root->bounds.extent = SpatialNodeTopLevelSize * SpatialNodePaddingRatio + SpatialNodeTopLevelSize;
	return root;
}

static sf_noinline void initializeSpatialChildren(SpatialNodeImp *node)
{
	node->children = sf::unique<SpatialChildren>();
	float childUnpaddedExtent = node->unpaddedExtent * 0.5f;
	float childMaxLeafExtent = node->maxLeafExtent * 0.5f;
	float childMinLeafExtent = childMaxLeafExtent * 0.5f;
	float childExtent = childUnpaddedExtent + childMaxLeafExtent;
	uint32_t childDepth = node->depth + 1;
	if (childDepth == SpatialNodeMaxDepth) {
		childMinLeafExtent = -HUGE_VALF;
	}

	sf::Vec3 origin = node->bounds.origin;

	SpatialNodeImp *children = node->children->child;
	for (uint32_t i = 0; i < 8; i++) {
		SpatialNodeImp &child = children[i];

		child.unpaddedExtent = childUnpaddedExtent;
		child.maxLeafExtent = childMaxLeafExtent;
		child.minLeafExtent = childMinLeafExtent;
		child.childMask = 0;
		child.depth = childDepth;
		child.bounds.origin.x = origin.x + (i & 1) ? -childUnpaddedExtent : childUnpaddedExtent;
		child.bounds.origin.y = origin.y + (i & 2) ? -childUnpaddedExtent : childUnpaddedExtent;
		child.bounds.origin.z = origin.z + (i & 4) ? -childUnpaddedExtent : childUnpaddedExtent;
		child.bounds.extent = childExtent;
	}
}

static SpatialNodeImp *insertSpatialNodeToRoot(SpatialNodeImp *rootNode, const sf::Vec3 &origin, float extent)
{
	SpatialNodeImp *node = rootNode;
	for (;;) {
		if (extent < node->minLeafExtent) {
			if (!node->children) initializeSpatialChildren(node);
			sf::Vec3 delta = origin - node->bounds.origin;
			uint32_t childIx = 0;
			childIx |= delta.x < 0.0f ? 1 : 0;
			childIx |= delta.y < 0.0f ? 2 : 0;
			childIx |= delta.z < 0.0f ? 4 : 0;
			node->childMask |= childIx;
			node = &node->children->child[childIx];
		} else {
			return node;
		}
	}
}

sf_inline sf::Vec3i getRootSpatialKey(const sf::Vec3 &origin)
{
	return sf::Vec3i(sf::floor((origin - sf::Vec3(SpatialNodeTopLevelSize * 0.5f)) * (1.0f / SpatialNodeTopLevelSize)));
}

static SpatialNodeImp *insertSpatialNode(AreaStateImp *imp, const sf::Vec3 &origin, float extent)
{
	sf::Vec3i key = getRootSpatialKey(origin);
	auto &result = imp->topSpatialNodes.insert(key);
	if (result.inserted) {
		result.entry.val = initializeSpatialRoot(key);
	}
	return insertSpatialNodeToRoot(result.entry.val, origin, extent);
}

static sf_forceinline bool checkSpatialNodeValid(const SpatialNodeImp *spatialNode, const sf::Vec3 &origin, float extent)
{
	if (extent > spatialNode->maxLeafExtent) return false;
	if (extent < spatialNode->minLeafExtent) return false;
	sf::Vec3 delta = sf::abs(origin - spatialNode->bounds.origin);
	float maxDelta = sf::max(delta.x, delta.y, delta.z);
	return maxDelta <= spatialNode->bounds.extent;
}

static sf_forceinline uint32_t addSpatialBox(SpatialNodeImp *spatialNode, uint32_t areaId, uint32_t groupId, uint32_t userId, uint32_t entityId, const sf::Bounds3 &bounds)
{
	uint32_t index = spatialNode->boxes.size;
	BoxArea &area = spatialNode->boxes.push();
	area.areaId = areaId;
	area.groupId = groupId;
	area.userId = userId;
	area.entityId = entityId;
	area.bounds = bounds;
	return index;
}

static sf_forceinline uint32_t addSpatialSphere(SpatialNodeImp *spatialNode, uint32_t areaId, uint32_t groupId, uint32_t userId, uint32_t entityId, const sf::Sphere &bounds)
{
	uint32_t index = spatialNode->spheres.size;
	SphereArea &area = spatialNode->spheres.push();
	area.areaId = areaId;
	area.groupId = groupId;
	area.userId = userId;
	area.entityId = entityId;
	area.bounds = bounds;
	return index;
}

static sf_forceinline void removeSpatialBox(AreaStateImp *imp, SpatialNodeImp *spatialNode, uint32_t index)
{
	BoxArea &swapBox = spatialNode->boxes.back();
	imp->areas[swapBox.areaId].spatialIndex = index;
	spatialNode->boxes.removeSwap(index);
}

static sf_forceinline void removeSpatialSphere(AreaStateImp *imp, SpatialNodeImp *spatialNode, uint32_t index)
{
	SphereArea &swapSphere = spatialNode->spheres.back();
	imp->areas[swapSphere.areaId].spatialIndex = index;
	spatialNode->spheres.removeSwap(index);
}

static sf_forceinline void removeEntityBoxImp(AreaStateImp *imp, uint32_t entityId, uint32_t areaId)
{
	EntityAreasImp *entityAreas = imp->entityAreas.findValue(entityId);
	sf_assert(entityAreas);
	bool found = sf::findRemoveSwap(entityAreas->boxes, areaId);
	sf_assert(found);
	if (entityAreas->boxes.size + entityAreas->spheres.size == 0) {
		imp->entityAreas.remove(entityId);
	}
}

static sf_forceinline void removeEntitySphereImp(AreaStateImp *imp, uint32_t entityId, uint32_t areaId)
{
	EntityAreasImp *entityAreas = imp->entityAreas.findValue(entityId);
	sf_assert(entityAreas);
	bool found = sf::findRemoveSwap(entityAreas->spheres, areaId);
	sf_assert(found);
	if (entityAreas->boxes.size + entityAreas->spheres.size == 0) {
		imp->entityAreas.remove(entityId);
	}
}

static sf_forceinline uint32_t addBoxImp(AreaStateImp *imp, const sf::Bounds3 &bounds, uint32_t groupId, uint32_t userId, uint32_t entityId, uint32_t attachEntityId)
{
	uint32_t areaId = imp->allocateAreaId();
	AreaImp &area = imp->areas[areaId];
	float extent = sf::max(bounds.extent.x, bounds.extent.y, bounds.extent.z);
	SpatialNodeImp *spatialNode = insertSpatialNode(imp, bounds.origin, extent);
	area.attachEntityId = attachEntityId;
	area.spatialNode = spatialNode;
	area.spatialIndex = addSpatialBox(spatialNode, areaId, groupId, userId, entityId, bounds);
	area.groupId = groupId;
	area.userId = userId;
	area.entityId = entityId;
	return areaId;
}

static sf_forceinline void updateBoxImp(AreaStateImp *imp, uint32_t areaId, const sf::Bounds3 &bounds)
{
	AreaImp &area = imp->areas[areaId];
	float extent = sf::max(bounds.extent.x, bounds.extent.y, bounds.extent.z);
	SpatialNodeImp *spatialNode = area.spatialNode;
	uint32_t spatialIndex = area.spatialIndex;
	BoxArea &box = spatialNode->boxes[spatialIndex];
	if (checkSpatialNodeValid(area.spatialNode, bounds.origin, extent)) {
		box.bounds = bounds;
	} else {
		uint32_t groupId = box.groupId, userId = box.userId;
		removeSpatialBox(imp, spatialNode, spatialIndex);
		spatialNode = insertSpatialNode(imp, bounds.origin, extent);
		area.spatialNode = spatialNode;
		area.spatialIndex = addSpatialBox(spatialNode, areaId, groupId, userId, area.entityId, bounds);
	}
}

static sf_forceinline uint32_t addSphereImp(AreaStateImp *imp, const sf::Sphere &bounds, uint32_t groupId, uint32_t userId, uint32_t entityId, uint32_t attachEntityId)
{
	uint32_t areaId = imp->allocateAreaId();
	AreaImp &area = imp->areas[areaId];
	float extent = bounds.radius;
	SpatialNodeImp *spatialNode = insertSpatialNode(imp, bounds.origin, extent);
	area.attachEntityId = attachEntityId;
	area.spatialNode = spatialNode;
	area.spatialIndex = addSpatialSphere(spatialNode, areaId, groupId, userId, entityId, bounds);
	area.groupId = groupId;
	area.userId = userId;
	area.entityId = entityId;
	return areaId;
}

static sf_forceinline void updateSphereImp(AreaStateImp *imp, uint32_t areaId, const sf::Sphere &bounds)
{
	AreaImp &area = imp->areas[areaId];
	float extent = bounds.radius;
	SpatialNodeImp *spatialNode = area.spatialNode;
	uint32_t spatialIndex = area.spatialIndex;
	SphereArea &sphere = spatialNode->spheres[spatialIndex];
	if (checkSpatialNodeValid(area.spatialNode, bounds.origin, extent)) {
		sphere.bounds = bounds;
	} else {
		uint32_t groupId = sphere.groupId, userId = sphere.userId;
		removeSpatialSphere(imp, spatialNode, spatialIndex);
		spatialNode = insertSpatialNode(imp, bounds.origin, extent);
		area.spatialNode = spatialNode;
		area.spatialIndex = addSpatialSphere(spatialNode, areaId, groupId, userId, area.entityId, bounds);
	}
}

sf::Box<AreaState> AreaState::create()
{
	sf::Box<AreaStateImp> box = sf::box<AreaStateImp>();
	box->groups.resize(AreaGroup_Count);
	return box;
}

uint32_t AreaState::addGroup()
{
	AreaStateImp *imp = (AreaStateImp*)this;
	uint32_t index;
	if (imp->freeGroupIndices.size > 0) {
		index = imp->freeGroupIndices.popValue();
	} else {
		index = imp->groups.size;
		imp->groups.push();
	}
	return index;
}

void AreaState::removeGroup(uint32_t groupId)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	GroupImp &group = imp->groups[groupId];
	sf_assert(group.areas.size() == 0);
	sf::reset(imp->groups[groupId]);
	imp->freeGroupIndices.push(groupId);
}

uint32_t AreaState::addEntityBox(uint32_t groupId, uint32_t userId, uint32_t entityId, const sf::Bounds3 &localBounds)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	const Entity *entity = clientGlobal->clientState->entities.find(entityId);
	sf::Bounds3 bounds = sf::transformBounds(entity->state.transform, localBounds);
	uint32_t areaId = addBoxImp(imp, bounds, groupId, userId, entityId, entityId);
	imp->entityAreas[entityId].boxes.push(areaId);
	return areaId;
}

void AreaState::updateEntityBox(uint32_t areaId, const sf::Bounds3 &localBounds)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	AreaImp &area = imp->areas[areaId];
	const Entity *entity = clientGlobal->clientState->entities.find(area.attachEntityId);
	sf::Bounds3 bounds = sf::transformBounds(entity->state.transform, localBounds);
	imp->areaLocalBounds[areaId].box = localBounds;
	updateBoxImp(imp, areaId, bounds);
}

void AreaState::removeEntityBox(uint32_t areaId)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	AreaImp &area = imp->areas[areaId];
	removeSpatialBox(imp, area.spatialNode, area.spatialIndex);
	removeEntityBoxImp(imp, area.attachEntityId, areaId);
	imp->freeAreaIndices.push(areaId);
}

uint32_t AreaState::addWorldBox(uint32_t groupId, uint32_t userId, uint32_t entityId, const sf::Bounds3 &bounds)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	return addBoxImp(imp, bounds, groupId, userId, entityId, 0);
}

void AreaState::updateWorldBox(uint32_t areaId, const sf::Bounds3 &bounds)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	updateBoxImp(imp, areaId, bounds);
}

void AreaState::removeWorldBox(uint32_t areaId)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	AreaImp &area = imp->areas[areaId];
	removeSpatialBox(imp, area.spatialNode, area.spatialIndex);
	imp->freeAreaIndices.push(areaId);
}

void AreaState::updateAnyBox(uint32_t areaId, const sf::Bounds3 &localBounds)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	AreaImp &area = imp->areas[areaId];
	sf::Bounds3 bounds = localBounds;
	if (area.attachEntityId) {
		const Entity *entity = clientGlobal->clientState->entities.find(area.attachEntityId);
		bounds = sf::transformBounds(entity->state.transform, bounds);
		imp->areaLocalBounds[areaId].box = localBounds;
	}
	updateBoxImp(imp, areaId, bounds);
}

void AreaState::removeAnyBox(uint32_t areaId)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	AreaImp &area = imp->areas[areaId];
	removeSpatialBox(imp, area.spatialNode, area.spatialIndex);
	if (area.attachEntityId) {
		removeEntityBoxImp(imp, area.attachEntityId, areaId);
	}
	imp->freeAreaIndices.push(areaId);
}

uint32_t AreaState::addEntitySphere(uint32_t groupId, uint32_t userId, uint32_t entityId, const sf::Sphere &localBounds)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	const Entity *entity = clientGlobal->clientState->entities.find(entityId);
	sf::Sphere bounds = sf::transformSphere(entity->state.transform, localBounds);
	uint32_t areaId = addSphereImp(imp, bounds, groupId, userId, entityId, entityId);
	imp->entityAreas[entityId].boxes.push(areaId);
	imp->areaLocalBounds[areaId].sphere = localBounds;
	return areaId;
}

void AreaState::updateEntitySphere(uint32_t areaId, const sf::Sphere &localBounds)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	AreaImp &area = imp->areas[areaId];
	const Entity *entity = clientGlobal->clientState->entities.find(area.attachEntityId);
	sf::Sphere bounds = sf::transformSphere(entity->state.transform, localBounds);
	updateSphereImp(imp, areaId, bounds);
}

void AreaState::removeEntitySphere(uint32_t areaId)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	AreaImp &area = imp->areas[areaId];
	removeSpatialSphere(imp, area.spatialNode, area.spatialIndex);
	removeEntitySphereImp(imp, area.attachEntityId, areaId);
	imp->freeAreaIndices.push(areaId);
}

uint32_t AreaState::addWorldSphere(uint32_t groupId, uint32_t userId, uint32_t entityId, const sf::Sphere &bounds)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	return addSphereImp(imp, bounds, groupId, userId, entityId, 0);
}

void AreaState::updateWorldSphere(uint32_t areaId, const sf::Sphere &bounds)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	updateSphereImp(imp, areaId, bounds);
}

void AreaState::removeWorldSphere(uint32_t areaId)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	AreaImp &area = imp->areas[areaId];
	removeSpatialSphere(imp, area.spatialNode, area.spatialIndex);
	imp->freeAreaIndices.push(areaId);
}

void AreaState::updateAnySphere(uint32_t areaId, const sf::Sphere &localBounds)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	AreaImp &area = imp->areas[areaId];
	sf::Sphere bounds = localBounds;
	if (area.entityId) {
		const Entity *entity = clientGlobal->clientState->entities.find(area.entityId);
		bounds = sf::transformSphere(entity->state.transform, bounds);
		imp->areaLocalBounds[areaId].sphere = localBounds;
	}
	updateSphereImp(imp, areaId, bounds);
}

void AreaState::removeAnySphere(uint32_t areaId)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	AreaImp &area = imp->areas[areaId];
	removeSpatialSphere(imp, area.spatialNode, area.spatialIndex);
	if (area.entityId) {
		removeEntitySphereImp(imp, area.entityId, areaId);
	}
	imp->freeAreaIndices.push(areaId);
}

bool AreaState::isAreaVisible(uint32_t areaId) const
{
	const AreaStateImp *imp = (const AreaStateImp*)this;
	uint32_t bit = 1 << (areaId & 31u);
	uint32_t word = areaId >> 5u;
	return (imp->areaVisibility[word] & bit) != 0;
}

uint32_t AreaState::findArea(uint32_t groupId, uint32_t userId) const
{
	const AreaStateImp *imp = (const AreaStateImp*)this;
	return imp->groups[groupId].areas.findOne(userId, ~0u);
}

void AreaState::updateEntityTransform(uint32_t entityId, const sf::Mat34 &transform)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	EntityAreasImp *entityAreas = imp->entityAreas.findValue(entityId);
	if (!entityAreas) return;

	for (uint32_t areaId : entityAreas->boxes) {
		AreaImp &area = imp->areas[areaId];
		BoxArea &box = area.spatialNode->boxes[area.spatialIndex];
		sf::Bounds3 bounds = sf::transformBounds(transform, imp->areaLocalBounds[areaId].box);
		updateBoxImp(imp, areaId, bounds);
	}

	for (uint32_t areaId : entityAreas->spheres) {
		AreaImp &area = imp->areas[areaId];
		SphereArea &sphere = area.spatialNode->spheres[area.spatialIndex];
		sf::Sphere bounds = sf::transformSphere(transform, imp->areaLocalBounds[areaId].sphere);
		updateSphereImp(imp, areaId, bounds);
	}
}

void AreaState::querySpatialNodesFrustum(sf::Array<const SpatialNode*> &nodes, const sf::Frustum &frustum) const
{
	const AreaStateImp *imp = (const AreaStateImp*)this;
	for (auto &pair : imp->topSpatialNodes) {
		walkSpatialNode(nodes, pair.val, frustum);
	}
}

const AreaGroupState &AreaState::getGroupState(uint32_t groupId) const
{
	const AreaStateImp *imp = (const AreaStateImp*)this;
	return imp->groups[groupId].state;
}

void AreaState::updateMainVisibility(sf::Slice<const SpatialNode*> nodes, const sf::Frustum &frustum)
{
	AreaStateImp *imp = (AreaStateImp*)this;

	uint32_t *areaVisibility = imp->areaVisibility.data;
	for (const SpatialNode *node : nodes) {
		for (const BoxArea &box : node->boxes) {
			if (frustum.intersects(box.bounds)) {
				uint32_t areaId = box.areaId;
				uint32_t bit = 1 << (areaId & 31u);
				uint32_t word = areaId >> 5u;
				areaVisibility[word] |= bit;
			}
		}
		for (const SphereArea &sphere : node->spheres) {
			if (frustum.intersects(sphere.bounds)) {
				uint32_t areaId = sphere.areaId;
				uint32_t bit = 1 << (areaId & 31u);
				uint32_t word = areaId >> 5u;
				areaVisibility[word] |= bit;
			}
		}
	}

	for (GroupImp &group : imp->groups) {
		group.state.newInvisible.clear();
		group.state.newVisible.clear();
	}

	uint32_t numWords = imp->areaVisibility.size;
	uint32_t *prevVisibility = imp->prevAreaVisibility.data;
	for (uint32_t word = 0; word < numWords; word++) {
		uint32_t &next = areaVisibility[word];
		uint32_t &prev = prevVisibility[word];
		if (prev == next) continue;

		uint32_t newVisible = next & ~prev;
		uint32_t newInvisible = prev & ~next;
		prev = next;
		next = 0;

		while (newVisible) {
			uint32_t ix = mx_ctz32(newVisible);
			newVisible &= newVisible - 1;
			uint32_t areaId = (word << 5) | ix;
			AreaImp &area = imp->areas[areaId];
			AreaIds &ids = imp->groups[area.groupId].state.newVisible.push();
			ids.areaId = areaId;
			ids.groupId = area.groupId;
			ids.userId = area.userId;
			ids.entityId = area.entityId;
		}

		while (newInvisible) {
			uint32_t ix = mx_ctz32(newInvisible);
			newInvisible &= newInvisible - 1;
			uint32_t areaId = (word << 5) | ix;
			AreaImp &area = imp->areas[areaId];
			AreaIds &ids = imp->groups[area.groupId].state.newInvisible.push();
			ids.areaId = areaId;
			ids.groupId = area.groupId;
			ids.userId = area.userId;
			ids.entityId = area.entityId;
		}
	}

}

}
