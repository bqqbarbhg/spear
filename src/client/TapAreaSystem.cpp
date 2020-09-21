#include "TapAreaSystem.h"

#include "client/AreaSystem.h"

#include "game/DebugDraw.h"

#include "sf/Geometry.h"

namespace cl {

struct TapAreaSystemImp final : TapAreaSystem
{
	struct TapArea
	{
		uint32_t areaId = ~0u;
		uint32_t entityId;

		sf::Mat34 entityToWorld;
		sf::Mat34 worldToEntity;
		sf::Bounds3 bounds;
	};

	sf::Array<TapArea> tapAreas;
	sf::Array<uint32_t> freeTapAreaIds;

	sf::Array<uint32_t> loadQueue;

	// API

	void addTapArea(Systems &systems, uint32_t entityId, uint8_t componentIndex, const sv::TapAreaComponent &c, const Transform &transform) override
	{
		uint32_t tapAreaId = tapAreas.size;
		if (freeTapAreaIds.size > 0) {
			tapAreaId = freeTapAreaIds.popValue();
		} else {
			tapAreas.push();
		}

		TapArea &tapArea = tapAreas[tapAreaId];

		tapArea.bounds.origin = c.offset;
		tapArea.bounds.extent = c.extent;
		tapArea.entityId = entityId;
		tapArea.entityToWorld = transform.asMatrix();
		tapArea.worldToEntity = sf::inverse(tapArea.entityToWorld);

		systems.entities.addComponent(entityId, this, tapAreaId, 0, componentIndex, Entity::UpdateTransform);
		tapArea.areaId = systems.area->addBoxArea(AreaGroup::TapArea, tapAreaId, tapArea.bounds, tapArea.entityToWorld, Area::EditorPick|Area::GamePick);
	}

	void updateTransform(Systems &systems, uint32_t entityId, const EntityComponent &ec, const TransformUpdate &update) override
	{
		uint32_t tapAreaId = ec.userId;
		TapArea &tapArea = tapAreas[tapAreaId];

		tapArea.entityToWorld = update.entityToWorld;
		tapArea.worldToEntity = sf::inverse(update.entityToWorld);
		systems.area->updateBoxArea(tapArea.areaId, tapArea.bounds, tapArea.entityToWorld);
	}

	void remove(Systems &systems, uint32_t entityId, const EntityComponent &ec) override
	{
		uint32_t tapAreaId = ec.userId;
		TapArea &tapArea = tapAreas[tapAreaId];

		systems.area->removeBoxArea(tapArea.areaId);

		freeTapAreaIds.push(tapAreaId);
		sf::reset(tapArea);
	}

	void getHoveredTapAreas(sf::Array<HoveredTapArea> &hovered, const AreaSystem *areaSystem, const sf::Ray &ray) const override
	{
		sf::FastRay fastRay { ray };

		sf::SmallArray<Area, 64> areas;
		areaSystem->castRay(areas, Area::GamePick, fastRay);
		for (Area &area : areas) {
			if (area.group != AreaGroup::TapArea) continue;
			uint32_t tapAreaId = area.userId;
			const TapArea &tapArea = tapAreas[tapAreaId];

			sf::Ray localRay = sf::transformRay(tapArea.worldToEntity, ray);
			float t;
			if (sf::intersectRay(t, localRay, tapArea.bounds)) {
				HoveredTapArea &hover = hovered.push();
				hover.t = t;
				hover.entityId = tapArea.entityId;
			}
		}
	}

	void editorHighlight(Systems &systems, const EntityComponent &ec, EditorHighlight type) override
	{
		uint32_t tapAreaId = ec.userId;
		const TapArea &tapArea = tapAreas[tapAreaId];

		sf::Vec3 color;
		switch (type) {
		case EditorHighlight::Hover: color = sf::Vec3(0.3f, 0.2f, 0.2f); break;
		default: color = sf::Vec3(1.0f, 0.8f, 0.8f); break;
		}

		debugDrawBox(tapArea.bounds, tapArea.entityToWorld, color);
	}

	void editorPick(sf::Array<EntityHit> &hits, const sf::FastRay &ray, uint32_t userId) const override
	{
		uint32_t tapAreaId = userId;
		const TapArea &tapArea = tapAreas[tapAreaId];

		sf::Ray localRay = sf::transformRay(sf::inverse(tapArea.entityToWorld), ray.ray);
		float t;
		if (sf::intersectRay(t, localRay, tapArea.bounds)) {
			EntityHit &hit = hits.push();
			hit.entityId = tapArea.entityId;
			hit.t = t;
			hit.approximate = true;
		}
	}

};

sf::Box<TapAreaSystem> TapAreaSystem::create() { return sf::box<TapAreaSystemImp>(); }

}
