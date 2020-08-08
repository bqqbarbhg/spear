#include "AreaState.h"

#include "sf/Frustum.h"
#include "sf/Unique.h"
#include "sf/UintMap.h"

#include "client/ClientGlobal.h"
#include "client/ClientState.h"

#include "sf/ext/mx/mx_platform.h"

namespace cl {

static const float SpatialNodeTopLevelSize = 128.0f;
static const float SpatialNodePaddingRatio = 0.5f;
static const uint32_t SpatialNodeMaxDepth = 6;
static const sf::Vec3 SpatialNodeGridOrigin = sf::Vec3(0.0f, -2.0f, 0.0f);

struct SpatialChildren;

struct SpatialNodeImp : SpatialNode
{
	sf::Vec3 origin;
	float unpaddedExtent;
	float maxLeafExtent;
	float minLeafExtent;
	uint32_t depth;
	uint32_t childMask;
	sf::Unique<SpatialChildren> children;
	SpatialNodeImp *parent;
	uint32_t optimizeFrame = 0;
	uint32_t higestOptimizeFrame = 0;
	bool boundsDirty = false;
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
	uint32_t visibleIndex = ~0u;
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

	sf::Array<uint32_t> areaVisibility;
	sf::Array<uint32_t> prevAreaVisibility;

	sf::HashMap<sf::Vec3i, sf::Unique<SpatialNodeImp>> topSpatialNodes;
	sf::HashMap<uint32_t, EntityAreasImp> entityAreas;

	sf::Array<SpatialNodeImp*> dirtySpatialNodes[SpatialNodeMaxDepth];

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

sf_inline bool intesersectRayAabb(const sf::Vec3 &origin, const sf::Vec3 &rcpDir, const sf::Vec3 &min, const sf::Vec3 &max, float tMin)
{
	sf::Vec3 loT = (min - origin) * rcpDir;
	sf::Vec3 hiT = (max - origin) * rcpDir;
	sf::Vec3 minT = sf::min(loT, hiT);
	sf::Vec3 maxT = sf::max(loT, hiT);
	float t0 = sf::max(minT.x, minT.y, minT.z);
	float t1 = sf::min(maxT.x, maxT.y, maxT.z);
	return t0 < t1 && t1 >= tMin;
}

static void walkSpatialNodes(sf::Array<const SpatialNode*> &nodes, const sf::Frustum &frustum)
{
	for (uint32_t nodeI = 0; nodeI < nodes.size; nodeI++) {
		const SpatialNodeImp *node = (const SpatialNodeImp*)nodes[nodeI];

		const SpatialNodeImp *children = node->children->child;
		uint32_t childMask = node->childMask;
		while (childMask) {
			uint32_t index = mx_ctz32(childMask);
			childMask &= childMask - 1;

			const SpatialNodeImp *child = &children[index];
			if (frustum.intersects(sf::Bounds3::minMax(child->min, child->max))) {
				nodes.push(child);
			}
		}
	}
}

static void walkSpatialNodesRay(sf::Array<const SpatialNode*> &nodes, sf::Vec3 origin, sf::Vec3 rcpDir, float tMin)
{
	for (uint32_t nodeI = 0; nodeI < nodes.size; nodeI++) {
		const SpatialNodeImp *node = (const SpatialNodeImp*)nodes[nodeI];

		const SpatialNodeImp *children = node->children->child;
		uint32_t childMask = node->childMask;
		while (childMask) {
			uint32_t index = mx_ctz32(childMask);
			childMask &= childMask - 1;

			const SpatialNodeImp *child = &children[index];
			if (intesersectRayAabb(origin, rcpDir, child->min, child->max, tMin)) {
				nodes.push(child);
			}
		}
	}
}

static sf_noinline sf::Unique<SpatialNodeImp> initializeSpatialRoot(const sf::Vec3i &key)
{
	sf::Unique<SpatialNodeImp> root = sf::unique<SpatialNodeImp>();
	root->unpaddedExtent = SpatialNodeTopLevelSize;
	root->maxLeafExtent = SpatialNodeTopLevelSize * SpatialNodePaddingRatio;
	root->minLeafExtent = root->maxLeafExtent * 0.5f;
	root->childMask = 0;
	root->depth = 0;
	root->origin = sf::Vec3(key) * SpatialNodeTopLevelSize + SpatialNodeGridOrigin;
	root->parent = nullptr;
	root->min = sf::Vec3(+HUGE_VALF);
	root->max = sf::Vec3(-HUGE_VALF);
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
	if (childDepth + 1 == SpatialNodeMaxDepth) {
		childMinLeafExtent = -HUGE_VALF;
	}

	sf::Vec3 origin = node->origin;

	SpatialNodeImp *children = node->children->child;
	for (uint32_t i = 0; i < 8; i++) {
		SpatialNodeImp &child = children[i];

		child.unpaddedExtent = childUnpaddedExtent;
		child.maxLeafExtent = childMaxLeafExtent;
		child.minLeafExtent = childMinLeafExtent;
		child.childMask = 0;
		child.depth = childDepth;
		child.origin.x = origin.x + ((i & 1) != 0 ? -childUnpaddedExtent : childUnpaddedExtent);
		child.origin.y = origin.y + ((i & 2) != 0 ? -childUnpaddedExtent : childUnpaddedExtent);
		child.origin.z = origin.z + ((i & 4) != 0 ? -childUnpaddedExtent : childUnpaddedExtent);
		child.parent = node;
		child.min = sf::Vec3(+HUGE_VALF);
		child.max = sf::Vec3(-HUGE_VALF);
	}
}

static SpatialNodeImp *insertSpatialNodeToRoot(SpatialNodeImp *rootNode, const sf::Vec3 &origin, float extent)
{
	SpatialNodeImp *node = rootNode;
	for (;;) {
		if (extent < node->minLeafExtent) {
			if (!node->children) initializeSpatialChildren(node);
			sf::Vec3 delta = origin - node->origin;
			uint32_t childIx = 0;
			childIx |= delta.x < 0.0f ? 1 : 0;
			childIx |= delta.y < 0.0f ? 2 : 0;
			childIx |= delta.z < 0.0f ? 4 : 0;
			node->childMask |= 1 << childIx;
			node = &node->children->child[childIx];
		} else {
			return node;
		}
	}
}

sf_inline sf::Vec3i getRootSpatialKey(const sf::Vec3 &origin)
{
	return sf::Vec3i(sf::floor((origin - SpatialNodeGridOrigin) * (1.0f / SpatialNodeTopLevelSize)));
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
	sf::Vec3 delta = sf::abs(origin - spatialNode->origin);
	float maxDelta = sf::max(delta.x, delta.y, delta.z);
	return maxDelta <= spatialNode->unpaddedExtent + spatialNode->maxLeafExtent;
}

static sf_forceinline void updateSpatialBounds(AreaStateImp *imp, SpatialNodeImp *spatialNode, const sf::Vec3 &min, const sf::Vec3 &max)
{
	spatialNode->min = sf::min(spatialNode->min, min);
	spatialNode->max = sf::max(spatialNode->max, max);

	SpatialNodeImp *parent = spatialNode->parent;
	while (parent && !parent->boundsDirty) {
		parent->boundsDirty = true;
		imp->dirtySpatialNodes[parent->depth].push(parent);
		parent = parent->parent;
	}
}

static sf_forceinline uint32_t addSpatialBox(AreaStateImp *imp, SpatialNodeImp *spatialNode, uint32_t areaId, uint32_t groupId, uint32_t userId, uint32_t entityId, const sf::Bounds3 &bounds)
{
	uint32_t index = spatialNode->boxes.size;
	BoxArea &area = spatialNode->boxes.push();
	area.areaId = areaId;
	area.groupId = groupId;
	area.userId = userId;
	area.entityId = entityId;
	area.bounds = bounds;
	updateSpatialBounds(imp, spatialNode, bounds.origin - bounds.extent, bounds.origin + bounds.extent);
	return index;
}

static sf_forceinline uint32_t addSpatialSphere(AreaStateImp *imp, SpatialNodeImp *spatialNode, uint32_t areaId, uint32_t groupId, uint32_t userId, uint32_t entityId, const sf::Sphere &bounds)
{
	uint32_t index = spatialNode->spheres.size;
	SphereArea &area = spatialNode->spheres.push();
	area.areaId = areaId;
	area.groupId = groupId;
	area.userId = userId;
	area.entityId = entityId;
	area.bounds = bounds;
	updateSpatialBounds(imp, spatialNode, bounds.origin - sf::Vec3(bounds.radius), bounds.origin + sf::Vec3(bounds.radius));
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
	imp->groups[groupId].areas.insert(userId, areaId);
	AreaImp &area = imp->areas[areaId];
	float extent = sf::max(bounds.extent.x, bounds.extent.y, bounds.extent.z);
	SpatialNodeImp *spatialNode = insertSpatialNode(imp, bounds.origin, extent);
	area.attachEntityId = attachEntityId;
	area.spatialNode = spatialNode;
	area.spatialIndex = addSpatialBox(imp, spatialNode, areaId, groupId, userId, entityId, bounds);
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
	if (checkSpatialNodeValid(spatialNode, bounds.origin, extent)) {
		box.bounds = bounds;
		updateSpatialBounds(imp, spatialNode, bounds.origin - bounds.extent, bounds.origin + bounds.extent);
	} else {
		uint32_t groupId = box.groupId, userId = box.userId;
		removeSpatialBox(imp, spatialNode, spatialIndex);
		spatialNode = insertSpatialNode(imp, bounds.origin, extent);
		area.spatialNode = spatialNode;
		area.spatialIndex = addSpatialBox(imp, spatialNode, areaId, groupId, userId, area.entityId, bounds);
	}
}

static sf_forceinline uint32_t addSphereImp(AreaStateImp *imp, const sf::Sphere &bounds, uint32_t groupId, uint32_t userId, uint32_t entityId, uint32_t attachEntityId)
{
	uint32_t areaId = imp->allocateAreaId();
	imp->groups[groupId].areas.insert(userId, areaId);
	AreaImp &area = imp->areas[areaId];
	float extent = bounds.radius;
	SpatialNodeImp *spatialNode = insertSpatialNode(imp, bounds.origin, extent);
	area.attachEntityId = attachEntityId;
	area.spatialNode = spatialNode;
	area.spatialIndex = addSpatialSphere(imp, spatialNode, areaId, groupId, userId, entityId, bounds);
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
		updateSpatialBounds(imp, spatialNode, bounds.origin - sf::Vec3(bounds.radius), bounds.origin + sf::Vec3(bounds.radius));
	} else {
		uint32_t groupId = sphere.groupId, userId = sphere.userId;
		removeSpatialSphere(imp, spatialNode, spatialIndex);
		spatialNode = insertSpatialNode(imp, bounds.origin, extent);
		area.spatialNode = spatialNode;
		area.spatialIndex = addSpatialSphere(imp, spatialNode, areaId, groupId, userId, area.entityId, bounds);
	}
}

static void optimizeSpatialNodeImp(AreaStateImp *imp, SpatialNodeImp *rootNode, uint32_t frame)
{
	SpatialNodeImp *best = rootNode;
	uint32_t bestFrame = frame - rootNode->optimizeFrame;

	bool improved;
	do {
		improved = false;
		SpatialNodeImp *children = best->children->child;
		uint32_t childMask = best->childMask;
		while (childMask) {
			uint32_t index = mx_ctz32(childMask);
			childMask &= childMask - 1;

			SpatialNodeImp *child = &children[index];
			uint32_t childFrame = frame - child->higestOptimizeFrame;
			if (childFrame >= bestFrame) {
				bestFrame = childFrame;
				best = child;
				improved = true;
			}
		}
	} while (improved);

	best->optimizeFrame = frame;

	sf::Vec3 min = sf::Vec3(+HUGE_VALF);
	sf::Vec3 max = sf::Vec3(-HUGE_VALF);
	for (BoxArea &box : best->boxes) {
		min = sf::min(min, box.bounds.origin - box.bounds.extent);
		max = sf::max(max, box.bounds.origin + box.bounds.extent);
	}
	for (SphereArea &sphere : best->spheres) {
		min = sf::min(min, sphere.bounds.origin - sf::Vec3(sphere.bounds.radius));
		max = sf::max(max, sphere.bounds.origin + sf::Vec3(sphere.bounds.radius));
	}

	{
		uint32_t highestFrame = frame - best->optimizeFrame;
		SpatialNodeImp *children = best->children->child;
		uint32_t childMask = best->childMask;
		while (childMask) {
			uint32_t index = mx_ctz32(childMask);
			childMask &= childMask - 1;

			SpatialNodeImp *child = &children[index];
			min = sf::min(min, child->min);
			max = sf::max(max, child->max);
			highestFrame = sf::max(highestFrame, frame - child->higestOptimizeFrame);
		}

		best->higestOptimizeFrame = frame - highestFrame;
	}

	best->min = min;
	best->max = max;

	SpatialNodeImp *parent = best->parent;
	while (parent) {
		uint32_t highestFrame = frame - parent->optimizeFrame;
		SpatialNodeImp *children = parent->children->child;
		uint32_t childMask = parent->childMask;
		while (childMask) {
			uint32_t index = mx_ctz32(childMask);
			childMask &= childMask - 1;

			SpatialNodeImp *child = &children[index];
			highestFrame = sf::max(highestFrame, frame - child->higestOptimizeFrame);
		}

		parent->higestOptimizeFrame = frame - highestFrame;
		parent = parent->parent;
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
	clientGlobalState->registerEntity(entityId, Entity::Area);
	const Entity *entity = clientGlobalState->entities.find(entityId);
	sf::Bounds3 bounds = sf::transformBounds(entity->transform.asMatrix(), localBounds);
	uint32_t areaId = addBoxImp(imp, bounds, groupId, userId, entityId, entityId);
	imp->entityAreas[entityId].boxes.push(areaId);
	return areaId;
}

void AreaState::updateEntityBox(uint32_t areaId, const sf::Bounds3 &localBounds)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	AreaImp &area = imp->areas[areaId];
	const Entity *entity = clientGlobalState->entities.find(area.attachEntityId);
	sf::Bounds3 bounds = sf::transformBounds(entity->transform.asMatrix(), localBounds);
	imp->areaLocalBounds[areaId].box = localBounds;
	updateBoxImp(imp, areaId, bounds);
}

void AreaState::removeEntityBox(uint32_t areaId)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	AreaImp &area = imp->areas[areaId];
	imp->groups[area.groupId].areas.removePair(area.userId, areaId);
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
	imp->groups[area.groupId].areas.removePair(area.userId, areaId);
	removeSpatialBox(imp, area.spatialNode, area.spatialIndex);
	imp->freeAreaIndices.push(areaId);
}

void AreaState::updateAnyBox(uint32_t areaId, const sf::Bounds3 &localBounds)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	AreaImp &area = imp->areas[areaId];
	sf::Bounds3 bounds = localBounds;
	if (area.attachEntityId) {
		const Entity *entity = clientGlobalState->entities.find(area.attachEntityId);
		bounds = sf::transformBounds(entity->transform.asMatrix(), bounds);
		imp->areaLocalBounds[areaId].box = localBounds;
	}
	updateBoxImp(imp, areaId, bounds);
}

void AreaState::removeAnyBox(uint32_t areaId)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	AreaImp &area = imp->areas[areaId];
	imp->groups[area.groupId].areas.removePair(area.userId, areaId);
	removeSpatialBox(imp, area.spatialNode, area.spatialIndex);
	if (area.attachEntityId) {
		removeEntityBoxImp(imp, area.attachEntityId, areaId);
	}
	imp->freeAreaIndices.push(areaId);
}

uint32_t AreaState::addEntitySphere(uint32_t groupId, uint32_t userId, uint32_t entityId, const sf::Sphere &localBounds)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	clientGlobalState->registerEntity(entityId, Entity::Area);
	const Entity *entity = clientGlobalState->entities.find(entityId);
	sf::Sphere bounds = sf::transformSphere(entity->transform.asMatrix(), localBounds);
	uint32_t areaId = addSphereImp(imp, bounds, groupId, userId, entityId, entityId);
	imp->entityAreas[entityId].boxes.push(areaId);
	imp->areaLocalBounds[areaId].sphere = localBounds;
	return areaId;
}

void AreaState::updateEntitySphere(uint32_t areaId, const sf::Sphere &localBounds)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	AreaImp &area = imp->areas[areaId];
	const Entity *entity = clientGlobalState->entities.find(area.attachEntityId);
	sf::Sphere bounds = sf::transformSphere(entity->transform.asMatrix(), localBounds);
	updateSphereImp(imp, areaId, bounds);
}

void AreaState::removeEntitySphere(uint32_t areaId)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	AreaImp &area = imp->areas[areaId];
	imp->groups[area.groupId].areas.removePair(area.userId, areaId);
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
	imp->groups[area.groupId].areas.removePair(area.userId, areaId);
	removeSpatialSphere(imp, area.spatialNode, area.spatialIndex);
	imp->freeAreaIndices.push(areaId);
}

void AreaState::updateAnySphere(uint32_t areaId, const sf::Sphere &localBounds)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	AreaImp &area = imp->areas[areaId];
	sf::Sphere bounds = localBounds;
	if (area.entityId) {
		const Entity *entity = clientGlobalState->entities.find(area.entityId);
		bounds = sf::transformSphere(entity->transform.asMatrix(), bounds);
		imp->areaLocalBounds[areaId].sphere = localBounds;
	}
	updateSphereImp(imp, areaId, bounds);
}

void AreaState::removeAnySphere(uint32_t areaId)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	AreaImp &area = imp->areas[areaId];
	imp->groups[area.groupId].areas.removePair(area.userId, areaId);
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
	uint32_t areaId = imp->groups[groupId].areas.findOne(userId, ~0u);
	sf_assert(areaId != ~0u);
	return areaId;
}

void AreaState::updateEntityTransform(uint32_t entityId, const VisualTransform &transform, const sf::Mat34 &matrix)
{
	AreaStateImp *imp = (AreaStateImp*)this;
	EntityAreasImp *entityAreas = imp->entityAreas.findValue(entityId);
	if (!entityAreas) return;

	for (uint32_t areaId : entityAreas->boxes) {
		AreaImp &area = imp->areas[areaId];
		BoxArea &box = area.spatialNode->boxes[area.spatialIndex];
		sf::Bounds3 bounds = sf::transformBounds(matrix, imp->areaLocalBounds[areaId].box);
		updateBoxImp(imp, areaId, bounds);
	}

	for (uint32_t areaId : entityAreas->spheres) {
		AreaImp &area = imp->areas[areaId];
		SphereArea &sphere = area.spatialNode->spheres[area.spatialIndex];
		sf::Sphere bounds = sf::transformSphere(matrix, imp->areaLocalBounds[areaId].sphere);
		updateSphereImp(imp, areaId, bounds);
	}
}

void AreaState::optimizeSpatialNodes()
{
	AreaStateImp *imp = (AreaStateImp*)this;
	uint32_t frame = (uint32_t)clientGlobal->frameIndex;

	SpatialNodeImp *bestRoot = nullptr;
	uint32_t bestFrame = 0;

	for (auto &pair : imp->topSpatialNodes) {
		uint32_t rootFrame = frame - pair.val->higestOptimizeFrame;
		if (rootFrame > bestFrame) {
			bestFrame = rootFrame;
			bestRoot = pair.val;
		}
	}

	if (bestRoot) {
		optimizeSpatialNodeImp(imp, bestRoot, frame);
	}
}

void AreaState::processDirtyNodes()
{
	AreaStateImp *imp = (AreaStateImp*)this;

	for (uint32_t i = SpatialNodeMaxDepth; i > 0; i--) {
		sf::Array<SpatialNodeImp*> &dirtyNodes = imp->dirtySpatialNodes[i - 1];
		for (SpatialNodeImp *node : dirtyNodes) {

			const SpatialNodeImp *children = node->children->child;

			uint32_t childMask = node->childMask;

			sf::Vec3 min, max;
			if (childMask) {
				uint32_t index = mx_ctz32(childMask);
				childMask &= childMask - 1;
				const SpatialNodeImp *child = &children[index];
				min = child->min;
				max = child->max;
			} else {
				min = sf::Vec3(+HUGE_VALF);
				max = sf::Vec3(-HUGE_VALF);
			}

			while (childMask) {
				uint32_t index = mx_ctz32(childMask);
				childMask &= childMask - 1;

				const SpatialNodeImp *child = &children[index];
				min = sf::min(min, child->min);
				max = sf::max(max, child->max);
			}

			node->min = min;
			node->max = max;
			node->boundsDirty = false;
		}
		dirtyNodes.clear();
	}
}

void AreaState::querySpatialNodesFrustum(sf::Array<const SpatialNode*> &nodes, const sf::Frustum &frustum)
{
	AreaStateImp *imp = (AreaStateImp*)this;

	processDirtyNodes();

	for (auto &pair : imp->topSpatialNodes) {
		if (frustum.intersects(sf::Bounds3::minMax(pair.val->min, pair.val->max))) {
			nodes.push(pair.val);
		}
	}
	walkSpatialNodes(nodes, frustum);
}

void AreaState::querySpatialNodesRay(sf::Array<const SpatialNode*> &nodes, const sf::Ray &ray, float tMin)
{
	AreaStateImp *imp = (AreaStateImp*)this;

	processDirtyNodes();

	sf::Vec3 origin = ray.origin;
	sf::Vec3 rcpDir = sf::Vec3(1.0f) / ray.direction;

	for (auto &pair : imp->topSpatialNodes) {
		if (intesersectRayAabb(origin, rcpDir, pair.val->min, pair.val->max, tMin)) {
			nodes.push(pair.val);
		}
	}
	walkSpatialNodesRay(nodes, origin, rcpDir, tMin);
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
			GroupImp &group = imp->groups[area.groupId];

			{
				AreaIds &ids = group.state.newVisible.push();
				ids.areaId = areaId;
				ids.groupId = area.groupId;
				ids.userId = area.userId;
				ids.entityId = area.entityId;
			}

			{
				area.visibleIndex = group.state.visible.size;
				AreaIds &ids = group.state.visible.push();
				ids.areaId = areaId;
				ids.groupId = area.groupId;
				ids.userId = area.userId;
				ids.entityId = area.entityId;
			}
		}

		while (newInvisible) {
			uint32_t ix = mx_ctz32(newInvisible);
			newInvisible &= newInvisible - 1;
			uint32_t areaId = (word << 5) | ix;
			AreaImp &area = imp->areas[areaId];
			GroupImp &group = imp->groups[area.groupId];

			{
				AreaIds &ids = group.state.newInvisible.push();
				ids.areaId = areaId;
				ids.groupId = area.groupId;
				ids.userId = area.userId;
				ids.entityId = area.entityId;
			}

			if (area.visibleIndex != ~0u) {
				imp->areas[group.state.visible.back().areaId].visibleIndex = area.visibleIndex;
				group.state.visible.removeSwap(area.visibleIndex);
				area.visibleIndex = ~0u;
			}
		}
	}

}

}
