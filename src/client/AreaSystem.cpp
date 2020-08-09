#include "AreaSystem.h"

#include "sf/Unique.h"
#include "sf/HashMap.h"
#include "sf/Frustum.h"
#include "sf/Float4.h"

#include "sf/ext/mx/mx_platform.h"

namespace cl {

static const constexpr float AreaMinY = -2.0f;
static const constexpr float AreaMaxY = 8.0f;

static const constexpr float SpatialTopLevelSize = 128.0f;
static const constexpr float SpatialBottomLevelSize = 32.0f;
static const constexpr float SpatialPaddingRatio = 0.5f;
static const sf::Vec3 SpatialGridOrigin = sf::Vec3(0.0f, AreaMinY, 0.0f);

struct AreaSystemImp final : AreaSystem
{
	struct BoxAreaImp
	{
		sf::Bounds3 bounds;
		uint32_t areaId;
		Area area;
	};

	struct SpatialChildren;
	struct Spatial
	{
		Spatial	*parent = nullptr;

		// Static bounds
		sf::Vec3 staticOrigin;
		float unpaddedExtent;
		float minLeafExtent;
		float maxLeafExtent;

		// Dynamic AABB
		sf::Vec3 aabbMin; float aabbMinPad = 0.0f;
		sf::Vec3 aabbMax; float aabbMaxPad = 0.0f;

		// Leaf areas
		uint32_t boxFlags = 0;
		sf::Array<BoxAreaImp> boxes;

		// Child areas
		uint32_t childMask = 0;
		sf::Unique<SpatialChildren> children;

		sf_forceinline void expand(sf::Float4 min, sf::Float4 max)
		{
			Spatial *s = this;
			do {
				sf::Float4 oldMin = sf::Float4::loadu(s->aabbMin.v);
				sf::Float4 oldMax = sf::Float4::loadu(s->aabbMax.v);
				sf::Float4 newMin = oldMin.min(min);
				sf::Float4 newMax = oldMax.max(max);
				sf::Float4 diffMin = oldMin - newMin;
				sf::Float4 diffMax = newMax - oldMax;
				sf::Float4 diff = diffMin.max(diffMax);
				if (!diff.anyGreaterThanZero()) break;
				newMin.storeu(s->aabbMin.v);
				newMax.storeu(s->aabbMax.v);
			} while ((s = s->parent) != nullptr);
		}

		void expand(const sf::Bounds3 &bounds)
		{
			sf::Float4 origin = { bounds.origin.x, bounds.origin.y, bounds.origin.z, 0.0f };
			sf::Float4 extent = { bounds.extent.x, bounds.extent.y, bounds.extent.z, 0.0f };
			sf::Float4 min = origin - extent;
			sf::Float4 max = origin + extent;
			expand(min, max);
		}

		bool isValidLeaf(const sf::Vec3 &origin, float extent) const
		{
			if (extent < minLeafExtent || extent > maxLeafExtent) return false;
			float maxDelta = sf::max(sf::abs(origin.x - staticOrigin.x),
				sf::abs(origin.y - staticOrigin.y), sf::abs(origin.z - staticOrigin.y));
			return maxDelta <= unpaddedExtent;
		}
	};

	struct SpatialChildren
	{
		Spatial child[8];
	};

	struct AreaImp
	{
		Spatial *spatial = nullptr;
		uint32_t spatialIndex = ~0u;
	};

	sf::HashMap<sf::Vec3i, sf::Unique<Spatial>> spatialRoots;

	sf::Array<AreaImp> areaImps;
	sf::Array<uint32_t> freeAreaIds;

	static void queryFrustumSpatial(const Spatial *spatial, sf::Array<Area> &areas, uint32_t areaFlags, const sf::Frustum &frustum)
	{
		if (spatial->boxFlags & areaFlags) {
			for (const BoxAreaImp &box : spatial->boxes) {
				if ((box.area.flags & areaFlags) == 0) continue;
				if (!frustum.intersects(box.bounds)) continue;

				areas.push(box.area);
			}
		}

		Spatial *children = spatial->children->child;
		uint32_t childMask = spatial->childMask;
		while (childMask) {
			uint32_t ix = mx_ctz32(childMask);
			childMask &= childMask - 1;

			Spatial *child = children + ix;
			if (!frustum.intersects(sf::Bounds3::minMax(child->aabbMin, child->aabbMax))) continue;

			queryFrustumSpatial(child, areas, areaFlags, frustum);
		}
	}

	static void castRaySpatial(const Spatial *spatial, sf::Array<Area> &areas, uint32_t areaFlags, const sf::FastRay &ray, float tMin, float tMax)
	{
		if (spatial->boxFlags & areaFlags) {
			for (const BoxAreaImp &box : spatial->boxes) {
				if ((box.area.flags & areaFlags) == 0) continue;
				float t = sf::intersectRayFast(ray, box.bounds, tMin, tMax);
				if (t >= tMax) continue;

				areas.push(box.area);
			}
		}

		Spatial *children = spatial->children->child;
		uint32_t childMask = spatial->childMask;
		while (childMask) {
			uint32_t ix = mx_ctz32(childMask);
			childMask &= childMask - 1;

			Spatial *child = children + ix;
			float t = sf::intersectRayFastAabb(ray, child->aabbMin, child->aabbMax, tMin, tMax);
			if (t >= tMax) continue;

			castRaySpatial(child, areas, areaFlags, ray, tMin, tMax);
		}
	}

	static sf::Unique<Spatial> initializeSpatialRoot(const sf::Vec3i &key)
	{
		sf::Unique<Spatial> root = sf::unique<Spatial>();
		root->unpaddedExtent = SpatialTopLevelSize;
		root->maxLeafExtent = SpatialTopLevelSize * SpatialPaddingRatio;
		root->minLeafExtent = root->maxLeafExtent * 0.5f;
		root->staticOrigin = sf::Vec3(key) * SpatialTopLevelSize + SpatialGridOrigin;
		root->aabbMin = sf::Vec3(+HUGE_VALF);
		root->aabbMax = sf::Vec3(-HUGE_VALF);
		return root;
	}

	static void initializeSpatialChildren(Spatial *spatial)
	{
		spatial->children = sf::unique<SpatialChildren>();
		float childUnpaddedExtent = spatial->unpaddedExtent * 0.5f;
		float childMaxLeafExtent = spatial->maxLeafExtent * 0.5f;
		float childMinLeafExtent = childMaxLeafExtent * 0.5f;
		if (childUnpaddedExtent <= SpatialBottomLevelSize) {
			childMinLeafExtent = -HUGE_VALF;
		}

		sf::Vec3 origin = spatial->staticOrigin;

		Spatial *children = spatial->children->child;
		for (uint32_t i = 0; i < 8; i++) {
			Spatial &child = children[i];

			child.unpaddedExtent = childUnpaddedExtent;
			child.maxLeafExtent = childMaxLeafExtent;
			child.minLeafExtent = childMinLeafExtent;
			child.staticOrigin.x = origin.x + ((i & 1) != 0 ? -childUnpaddedExtent : childUnpaddedExtent);
			child.staticOrigin.y = origin.y + ((i & 2) != 0 ? -childUnpaddedExtent : childUnpaddedExtent);
			child.staticOrigin.z = origin.z + ((i & 4) != 0 ? -childUnpaddedExtent : childUnpaddedExtent);
			child.parent = spatial;
			child.aabbMin = sf::Vec3(+HUGE_VALF);
			child.aabbMax = sf::Vec3(-HUGE_VALF);
		}
	}

	Spatial *insertSpatial(const sf::Vec3 &origin, float extent)
	{
		sf::Vec3i key = sf::Vec3i(sf::floor((origin - SpatialGridOrigin) * (1.0f / SpatialTopLevelSize)));
		auto &result = spatialRoots.insert(key);
		if (result.inserted) {
			result.entry.val = initializeSpatialRoot(key);
		}

		Spatial *spatial = result.entry.val;
		for (;;) {
			if (extent < spatial->minLeafExtent) {
				if (!spatial->children) initializeSpatialChildren(spatial);
				sf::Vec3 delta = origin - spatial->staticOrigin;
				uint32_t childIx = 0;
				childIx |= delta.x < 0.0f ? 1 : 0;
				childIx |= delta.y < 0.0f ? 2 : 0;
				childIx |= delta.z < 0.0f ? 4 : 0;
				spatial->childMask |= 1 << childIx;
				spatial = &spatial->children->child[childIx];
			} else {
				return spatial;
			}
		}
	}

	static sf_forceinline sf::Bounds3 clampBounds(const sf::Bounds3 &bounds)
	{
		sf::Bounds3 result;
		result.origin.x = bounds.origin.x;
		result.origin.z = bounds.origin.z;
		result.extent.x = bounds.extent.x;
		result.extent.z = bounds.extent.z;
		float minY = sf::clamp(bounds.origin.y - bounds.extent.y, AreaMinY, AreaMaxY);
		float maxY = sf::clamp(bounds.origin.y + bounds.extent.y, AreaMinY, AreaMaxY);
		result.origin.y = (minY + maxY) * 0.5f;
		result.extent.y = (maxY - minY) * 0.5f;
		return result;
	}

	// API

	uint32_t addBoxArea(AreaGroup group, uint32_t userId, const sf::Bounds3 &unclampedBounds, uint32_t areaFlags)
	{
		uint32_t areaId = areaImps.size;
		if (freeAreaIds.size > 0) {
			areaId = freeAreaIds.popValue();
		} else {
			areaImps.push();
		}

		sf::Bounds3 bounds = clampBounds(unclampedBounds);

		AreaImp &areaImp = areaImps[areaId];
		float extent = sf::max(bounds.extent.x, bounds.extent.y, bounds.extent.z);
		Spatial *spatial = insertSpatial(bounds.origin, extent);
		spatial->boxFlags |= areaFlags;
		areaImp.spatial = spatial;
		areaImp.spatialIndex = spatial->boxes.size;
		BoxAreaImp &box = spatial->boxes.push();
		box.bounds = bounds;
		box.areaId = areaId;
		box.area.group = group;
		box.area.flags = areaFlags;
		box.area.userId = userId;
		spatial->expand(bounds);

		return areaId;
	}

	uint32_t addBoxArea(AreaGroup group, uint32_t userId, const sf::Bounds3 &bounds, const sf::Mat34 &transform, uint32_t areaFlags)
	{
		// TODO: OBB support?
		return addBoxArea(group, userId, sf::transformBounds(transform, bounds), areaFlags);
	}

	void updateBoxArea(uint32_t areaId, const sf::Bounds3 &unclampedBounds)
	{
		AreaImp &areaImp = areaImps[areaId];

		sf::Bounds3 bounds = clampBounds(unclampedBounds);

		float extent = sf::max(bounds.extent.x, bounds.extent.y, bounds.extent.z);
		Spatial *spatial = areaImp.spatial;
		uint32_t spatialIndex = areaImp.spatialIndex;
		if (spatial->isValidLeaf(bounds.origin, extent)) {
			spatial->boxes[spatialIndex].bounds = bounds;
			spatial->expand(bounds);
		} else {
			Area area = spatial->boxes[spatialIndex].area;

			areaImps[spatial->boxes.back().areaId].spatialIndex = spatialIndex;
			spatial->boxes.removeSwap(spatialIndex);

			spatial = insertSpatial(bounds.origin, extent);
			spatial->boxFlags |= area.flags;
			areaImp.spatial = spatial;
			areaImp.spatialIndex = spatial->boxes.size;
			BoxAreaImp &box = spatial->boxes.push();
			box.bounds = bounds;
			box.areaId = areaId;
			box.area = area;
		}
	}

	void updateBoxArea(uint32_t areaId, const sf::Bounds3 &bounds, const sf::Mat34 &transform)
	{
		// TODO: OBB support?
		updateBoxArea(areaId, sf::transformBounds(transform, bounds));
	}

	void removeBoxArea(uint32_t areaId)
	{
		AreaImp &areaImp = areaImps[areaId];
		Spatial *spatial = areaImp.spatial;
		uint32_t spatialIndex = areaImp.spatialIndex;
		areaImps[spatial->boxes.back().areaId].spatialIndex = spatialIndex;
		spatial->boxes.removeSwap(spatialIndex);

		sf::reset(areaImp);
		freeAreaIds.push(areaId);
	}

	void optimize()
	{
	}

	void queryFrustum(sf::Array<Area> &areas, uint32_t areaFlags, const sf::Frustum &frustum) const
	{
		sf::Frustum local = frustum;
		for (const auto &root : spatialRoots) {
			const Spatial *spatial = root.val;
			if (!frustum.intersects(sf::Bounds3::minMax(spatial->aabbMin, spatial->aabbMax))) continue;

			queryFrustumSpatial(spatial, areas, areaFlags, frustum);
		}
	}

	void castRay(sf::Array<Area> &areas, uint32_t areaFlags, const sf::Ray &ray, float tMin, float tMax) const
	{
		sf::FastRay fastRay { ray };
		for (const auto &root : spatialRoots) {
			const Spatial *spatial = root.val;
			float t = sf::intersectRayFastAabb(fastRay, spatial->aabbMin, spatial->aabbMax, tMin, tMax);
			if (t >= tMax) continue;

			castRaySpatial(spatial, areas, areaFlags, fastRay, tMin, tMax);
		}
	}
};

sf::Box<AreaSystem> AreaSystem::create() { return sf::box<AreaSystemImp>(); }

}
