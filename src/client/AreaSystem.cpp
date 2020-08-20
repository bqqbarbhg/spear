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
static const constexpr float SpatialPaddingRatio = 0.5f;
static const constexpr uint32_t SpatialMaxDepth = 7;
static const constexpr uint32_t SpatialMinOptimizeDepth = 5;
static const sf::Vec3 SpatialGridOrigin = sf::Vec3(0.0f, AreaMinY, 0.0f);

struct AreaSystemImp final : AreaSystem
{
	struct BoxAreaImp
	{
		sf::Bounds3 bounds;
		uint32_t areaId;
		Area area;
	};

	struct SphereAreaImp
	{
		sf::Sphere sphere;
		uint32_t areaId;
		Area area;
	};

	struct SpatialChildren;
	struct Spatial
	{
		Spatial	*parent = nullptr;

		// TODO: Pack this somewhere
		bool inOptimizationQueue = false;
		uint8_t depth = 0;

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
		uint32_t sphereFlags = 0;
		sf::Array<SphereAreaImp> spheres;

		// Child areas
		uint32_t childMask = 0;
		sf::Unique<SpatialChildren> children;

		sf_forceinline void expandImp(sf::Float4 min, sf::Float4 max, uint32_t level)
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
				s = s->parent;
			} while (--level >= SpatialMinOptimizeDepth);
		}

		void expand(const sf::Bounds3 &bounds)
		{
			uint32_t level = (uint32_t)depth;
			if (level < SpatialMinOptimizeDepth) return;

			sf::Float4 origin = { bounds.origin.x, bounds.origin.y, bounds.origin.z, 0.0f };
			sf::Float4 extent = { bounds.extent.x, bounds.extent.y, bounds.extent.z, 0.0f };
			sf::Float4 min = origin - extent;
			sf::Float4 max = origin + extent;
			expandImp(min, max, level);
		}

		bool isValidLeaf(const sf::Vec3 &origin, float extent) const
		{
			if (extent < minLeafExtent || extent > maxLeafExtent) return false;
			float maxDelta = sf::max(sf::abs(origin.x - staticOrigin.x),
				sf::abs(origin.y - staticOrigin.y), sf::abs(origin.z - staticOrigin.z));
			return maxDelta <= unpaddedExtent;
		}
	};

	struct SpatialChildren
	{
		Spatial child[8];
	};

	enum class AreaShape
	{
		Invalid,
		Box,
		Sphere,
	};

	struct AreaImp
	{
		Spatial *spatial = nullptr;
		uint32_t spatialIndex = ~0u;

		#if SF_DEBUG
			AreaShape debugAreaShape = AreaShape::Invalid;
			void setShape(AreaShape shape) { debugAreaShape = shape; }
			void checkShape(AreaShape shape) { sf_assert(debugAreaShape == shape); }
		#else
			sf_forceinline void setShape(AreaShape shape) { }
			sf_forceinline void checkShape(AreaShape shape) { }
		#endif
	};

	sf::HashMap<sf::Vec3i, sf::Unique<Spatial>> spatialRoots;

	sf::Array<AreaImp> areaImps;
	sf::Array<uint32_t> freeAreaIds;
	sf::Array<Spatial*> optimizationQueues[SpatialMaxDepth];
	uint32_t optimizationQueueOffsets[SpatialMaxDepth] = { };

	static void queryFrustumSpatial(const Spatial *spatial, sf::Array<Area> &areas, uint32_t areaFlags, const sf::Frustum &frustum)
	{
		if (spatial->boxFlags & areaFlags) {
			for (const BoxAreaImp &box : spatial->boxes) {
				if ((box.area.flags & areaFlags) == 0) continue;
				if (!frustum.intersects(box.bounds)) continue;

				areas.push(box.area);
			}
		}

		if (spatial->sphereFlags & areaFlags) {
			for (const SphereAreaImp &sph : spatial->spheres) {
				if ((sph.area.flags & areaFlags) == 0) continue;
				if (!frustum.intersects(sph.sphere)) continue;

				areas.push(sph.area);
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

	static void queryFrustumSpatialBounds(const Spatial *spatial, sf::Array<AreaBounds> &areas, uint32_t areaFlags, const sf::Frustum &frustum)
	{
		if (spatial->boxFlags & areaFlags) {
			for (const BoxAreaImp &box : spatial->boxes) {
				if ((box.area.flags & areaFlags) == 0) continue;
				if (!frustum.intersects(box.bounds)) continue;

				AreaBounds &area = areas.push();
				area.area = box.area;
				area.bounds = box.bounds;
			}
		}

		if (spatial->sphereFlags & areaFlags) {
			for (const SphereAreaImp &sph : spatial->spheres) {
				if ((sph.area.flags & areaFlags) == 0) continue;
				if (!frustum.intersects(sph.sphere)) continue;

				AreaBounds &area = areas.push();
				area.area = sph.area;
				area.bounds.origin = sph.sphere.origin;
				area.bounds.extent = sf::Vec3(sph.sphere.origin);
			}
		}

		Spatial *children = spatial->children->child;
		uint32_t childMask = spatial->childMask;
		while (childMask) {
			uint32_t ix = mx_ctz32(childMask);
			childMask &= childMask - 1;

			Spatial *child = children + ix;
			if (!frustum.intersects(sf::Bounds3::minMax(child->aabbMin, child->aabbMax))) continue;

			queryFrustumSpatialBounds(child, areas, areaFlags, frustum);
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

		if (spatial->sphereFlags & areaFlags) {
			for (const SphereAreaImp &sph : spatial->spheres) {
				if ((sph.area.flags & areaFlags) == 0) continue;
				float t = sf::intersectRayFast(ray, sph.sphere, tMin, tMax);
				if (t >= tMax) continue;

				areas.push(sph.area);
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

	sf_forceinline static void initializeSpatialOptimization(Spatial *spatial)
	{
		// Set to be sticky in optimization queue with static bounds
		if (spatial->depth < SpatialMinOptimizeDepth) {
			float extent = spatial->unpaddedExtent + spatial->maxLeafExtent;
			spatial->inOptimizationQueue = true;
			spatial->aabbMin = spatial->staticOrigin - sf::Vec3(extent);
			spatial->aabbMax = spatial->staticOrigin + sf::Vec3(extent);
			spatial->aabbMin.y = sf::clamp(spatial->aabbMin.y, AreaMinY, AreaMaxY);
			spatial->aabbMax.y = sf::clamp(spatial->aabbMax.y, AreaMinY, AreaMaxY);
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
		initializeSpatialOptimization(root);

		// HACK: Clear the maximum extent to allow huge objects into the last level
		root->maxLeafExtent = HUGE_VALF;

		return root;
	}

	static void initializeSpatialChildren(Spatial *spatial)
	{
		spatial->children = sf::unique<SpatialChildren>();
		float childUnpaddedExtent = spatial->unpaddedExtent * 0.5f;
		float childMaxLeafExtent = spatial->minLeafExtent;
		float childMinLeafExtent = childMaxLeafExtent * 0.5f;
		uint8_t childDepth = (uint8_t)(spatial->depth + 1);
		if (childDepth + 1 == SpatialMaxDepth) {
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
			child.depth = childDepth;
			initializeSpatialOptimization(&child);
		}
	}

	Spatial *insertSpatial(const sf::Vec3 &origin, float extent)
	{
		sf::Vec3i key = sf::Vec3i(sf::floor((origin - SpatialGridOrigin) * (1.0f / SpatialTopLevelSize)));
		auto result = spatialRoots.insert(key);
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

	static sf_forceinline sf::Bounds3 clampBounds(const sf::Sphere &sphere)
	{
		sf::Bounds3 result;
		result.origin.x = sphere.origin.x;
		result.origin.z = sphere.origin.z;
		result.extent.x = sphere.radius;
		result.extent.z = sphere.radius;
		float minY = sf::clamp(sphere.origin.y - sphere.radius, AreaMinY, AreaMaxY);
		float maxY = sf::clamp(sphere.origin.y + sphere.radius, AreaMinY, AreaMaxY);
		result.origin.y = (minY + maxY) * 0.5f;
		result.extent.y = (maxY - minY) * 0.5f;
		return result;
	}

	sf_forceinline void addToOptimizationQueue(Spatial *spatial)
	{
		if (!spatial->inOptimizationQueue) {
			spatial->inOptimizationQueue = true;
			optimizationQueues[spatial->depth].push(spatial);
		}
	}

	void optimizeSpatial(Spatial *spatial)
	{
		if (spatial->parent) {
			addToOptimizationQueue(spatial->parent);
		}

		sf::Float4 aabbMin = +HUGE_VALF;
		sf::Float4 aabbMax = -HUGE_VALF;

		uint32_t boxFlags = 0;
		for (BoxAreaImp &box : spatial->boxes) {
			sf::Float4 origin = { box.bounds.origin.x, box.bounds.origin.y, box.bounds.origin.z, 0.0f };
			sf::Float4 extent = { box.bounds.extent.x, box.bounds.extent.y, box.bounds.extent.z, 0.0f };
			aabbMin = aabbMin.min(origin - extent);
			aabbMax = aabbMax.max(origin + extent);
			boxFlags |= box.area.flags;
		}
		spatial->boxFlags = boxFlags;

		uint32_t sphereFlags = 0;
		for (SphereAreaImp &sph : spatial->spheres) {
			sf::Float4 origin = { sph.sphere.origin.x, sph.sphere.origin.y, sph.sphere.origin.z, 0.0f };
			sf::Float4 extent = { sph.sphere.radius, sph.sphere.radius, sph.sphere.radius, 0.0f };
			aabbMin = aabbMin.min(origin - extent);
			aabbMax = aabbMax.max(origin + extent);
			sphereFlags |= sph.area.flags;
		}
		spatial->sphereFlags = sphereFlags;

		if (spatial->children) {
			uint32_t childMask = 0;
			uint32_t childBit = 1;
			for (Spatial &child : spatial->children->child) {
				if (child.childMask | child.boxFlags | child.sphereFlags) {
					childMask |= childBit;
					aabbMin = aabbMin.min(sf::Float4::loadu(child.aabbMin.v));
					aabbMax = aabbMax.max(sf::Float4::loadu(child.aabbMax.v));
				}
				childBit <<= 1;
			}
			spatial->childMask = childMask;
		} else {
			sf_assert(spatial->childMask == 0);
		}

		aabbMin.storeu(spatial->aabbMin.v);
		aabbMax.storeu(spatial->aabbMax.v);
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
		areaImp.spatial = spatial;
		areaImp.spatialIndex = spatial->boxes.size;
		areaImp.setShape(AreaShape::Box);

		spatial->boxFlags |= areaFlags;
		BoxAreaImp &box = spatial->boxes.push();
		box.bounds = bounds;
		box.areaId = areaId;
		box.area.group = group;
		box.area.flags = areaFlags;
		box.area.userId = userId;
		spatial->expand(bounds);

		addToOptimizationQueue(spatial);

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
		areaImp.checkShape(AreaShape::Box);

		sf::Bounds3 bounds = clampBounds(unclampedBounds);

		float extent = sf::max(bounds.extent.x, bounds.extent.y, bounds.extent.z);
		Spatial *spatial = areaImp.spatial;
		uint32_t spatialIndex = areaImp.spatialIndex;
		if (spatial->isValidLeaf(bounds.origin, extent)) {
			spatial->boxes[spatialIndex].bounds = bounds;
		} else {
			addToOptimizationQueue(spatial);

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
		spatial->expand(bounds);
		addToOptimizationQueue(spatial);
	}

	void updateBoxArea(uint32_t areaId, const sf::Bounds3 &bounds, const sf::Mat34 &transform)
	{
		// TODO: OBB support?
		updateBoxArea(areaId, sf::transformBounds(transform, bounds));
	}

	void removeBoxArea(uint32_t areaId)
	{
		AreaImp &areaImp = areaImps[areaId];
		areaImp.checkShape(AreaShape::Box);

		Spatial *spatial = areaImp.spatial;
		uint32_t spatialIndex = areaImp.spatialIndex;
		areaImps[spatial->boxes.back().areaId].spatialIndex = spatialIndex;
		spatial->boxes.removeSwap(spatialIndex);

		addToOptimizationQueue(spatial);

		sf::reset(areaImp);
		freeAreaIds.push(areaId);
	}

	uint32_t addSphereArea(AreaGroup group, uint32_t userId, const sf::Sphere &sphere, uint32_t areaFlags) override
	{
		uint32_t areaId = areaImps.size;
		if (freeAreaIds.size > 0) {
			areaId = freeAreaIds.popValue();
		} else {
			areaImps.push();
		}

		sf::Bounds3 bounds = clampBounds(sphere);

		float extent = sphere.radius;
		AreaImp &areaImp = areaImps[areaId];
		Spatial *spatial = insertSpatial(bounds.origin, extent);

		areaImp.spatial = spatial;
		areaImp.spatialIndex = spatial->spheres.size;
		areaImp.setShape(AreaShape::Sphere);

		spatial->sphereFlags |= areaFlags;
		SphereAreaImp &sph = spatial->spheres.push();
		sph.sphere = sphere;
		sph.areaId = areaId;
		sph.area.group = group;
		sph.area.flags = areaFlags;
		sph.area.userId = userId;
		spatial->expand(bounds);

		addToOptimizationQueue(spatial);

		return areaId;
	}

	uint32_t addSphereArea(AreaGroup group, uint32_t userId, const sf::Sphere &sphere, const sf::Mat34 &transform, uint32_t areaFlags) override
	{
		// TODO: OBB support?
		return addSphereArea(group, userId, sf::transformSphere(transform, sphere), areaFlags);
	}

	void updateSphereArea(uint32_t areaId, const sf::Sphere &sphere)
	{
		AreaImp &areaImp = areaImps[areaId];
		areaImp.checkShape(AreaShape::Sphere);

		sf::Bounds3 bounds = clampBounds(sphere);

		float extent = sphere.radius;
		Spatial *spatial = areaImp.spatial;
		uint32_t spatialIndex = areaImp.spatialIndex;
		if (spatial->isValidLeaf(bounds.origin, extent)) {
			spatial->spheres[spatialIndex].sphere = sphere;
		} else {
			addToOptimizationQueue(spatial);

			Area area = spatial->spheres[spatialIndex].area;

			areaImps[spatial->spheres.back().areaId].spatialIndex = spatialIndex;
			spatial->spheres.removeSwap(spatialIndex);

			spatial = insertSpatial(bounds.origin, extent);
			spatial->sphereFlags |= area.flags;
			areaImp.spatial = spatial;
			areaImp.spatialIndex = spatial->spheres.size;
			SphereAreaImp &sph = spatial->spheres.push();
			sph.sphere = sphere;
			sph.areaId = areaId;
			sph.area = area;
		}

		spatial->expand(bounds);
		addToOptimizationQueue(spatial);
	}

	void updateSphereArea(uint32_t areaId, const sf::Sphere &sphere, const sf::Mat34 &transform)
	{
		// TODO: OBB support?
		updateSphereArea(areaId, sf::transformSphere(transform, sphere));
	}

	void removeSphereArea(uint32_t areaId)
	{
		AreaImp &areaImp = areaImps[areaId];
		areaImp.checkShape(AreaShape::Sphere);

		Spatial *spatial = areaImp.spatial;
		uint32_t spatialIndex = areaImp.spatialIndex;
		areaImps[spatial->spheres.back().areaId].spatialIndex = spatialIndex;
		spatial->spheres.removeSwap(spatialIndex);

		addToOptimizationQueue(spatial);

		sf::reset(areaImp);
		freeAreaIds.push(areaId);
	}

	void optimize()
	{
		static const uint32_t updatesPerLevel[] = { 4, 2 };
		for (uint32_t invLevel = 0; invLevel < SpatialMaxDepth - SpatialMinOptimizeDepth; invLevel++) {
			uint32_t level = SpatialMaxDepth - invLevel - 1;
			uint32_t updates = 1;
			if (invLevel < sf_arraysize(updatesPerLevel)) {
				updates = updatesPerLevel[invLevel];
			}

			sf::Array<Spatial*> &queue = optimizationQueues[level];
			uint32_t &offset = optimizationQueueOffsets[level];

			while (updates-- > 0) {
				if (offset == queue.size) break;

				Spatial *spatial = queue[offset];
				spatial->inOptimizationQueue = false;

				optimizeSpatial(spatial);
				offset++;
			}

			if (offset >= 512) {
				memmove(queue.data, queue.data + offset, (queue.size - offset) * sizeof(Spatial*));
				queue.size -= offset;
				offset = 0;
			}
		}
	}

	void queryFrustum(sf::Array<Area> &areas, uint32_t areaFlags, const sf::Frustum &frustum) const override
	{
		sf::Frustum local = frustum;
		for (const auto &root : spatialRoots) {
			const Spatial *spatial = root.val;
			if ((spatial->boxFlags | spatial->sphereFlags | spatial->childMask) == 0) continue;
			if (!frustum.intersects(sf::Bounds3::minMax(spatial->aabbMin, spatial->aabbMax))) continue;

			queryFrustumSpatial(spatial, areas, areaFlags, frustum);
		}
	}

	void queryFrustumBounds(sf::Array<AreaBounds> &areas, uint32_t areaFlags, const sf::Frustum &frustum) const override
	{
		sf::Frustum local = frustum;
		for (const auto &root : spatialRoots) {
			const Spatial *spatial = root.val;
			if ((spatial->boxFlags | spatial->sphereFlags | spatial->childMask) == 0) continue;
			if (!frustum.intersects(sf::Bounds3::minMax(spatial->aabbMin, spatial->aabbMax))) continue;

			queryFrustumSpatialBounds(spatial, areas, areaFlags, frustum);
		}
	}

	void castRay(sf::Array<Area> &areas, uint32_t areaFlags, const sf::FastRay &ray, float tMin, float tMax) const override
	{
		for (const auto &root : spatialRoots) {
			const Spatial *spatial = root.val;
			if ((spatial->boxFlags | spatial->sphereFlags | spatial->childMask) == 0) continue;

			float t = sf::intersectRayFastAabb(ray, spatial->aabbMin, spatial->aabbMax, tMin, tMax);
			if (t >= tMax) continue;

			castRaySpatial(spatial, areas, areaFlags, ray, tMin, tMax);
		}
	}
};

sf::Box<AreaSystem> AreaSystem::create() { return sf::box<AreaSystemImp>(); }

}
