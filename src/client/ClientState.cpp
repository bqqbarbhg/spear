#include "ClientState.h"

#include "client/AreaSystem.h"
#include "client/ModelSystem.h"
#include "sf/Frustum.h"

#include "sp/Renderer.h"

namespace cl {

static Transform getPropTransform(const sv::PropTransform &transform)
{
	Transform ret;
	ret.position = sf::Vec3((float)transform.tile.x, 0.0f, (float)transform.tile.y) + transform.visualOffset;
	ret.rotation = sf::eulerAnglesToQuat(transform.visualRotation);
	ret.scale = transform.scale;
	return ret;
}

static void updateVisibility(VisibleAreas &visibleAreas, const AreaSystem *areaSystem, const sf::Frustum &frustum)
{
	visibleAreas.areas.clear();
	areaSystem->queryFrustum(visibleAreas.areas, Area::Visibilty, frustum);

	for (sf::Array<uint32_t> &groupIds : visibleAreas.groups) {
		groupIds.clear();
	}

	for (const Area &area : visibleAreas.areas) {
		if ((uint32_t)area.group >= sf_arraysize(visibleAreas.groups)) continue;
		visibleAreas.groups[(uint32_t)area.group].push(area.userId);
	}
}

static void addEntityComponents(Systems &systems, uint32_t entityId, const Transform &transform, const Prefab &prefab)
{
	uint8_t compIx = 0;
	for (const sf::Box<sv::Component> &comp : prefab.s.components) {

		if (const auto *c = comp->as<sv::ModelComponent>()) {
			systems.model->addModel(systems, entityId, compIx, *c, transform);
		}

		compIx++;
	}
}


static uint32_t addEntity(Systems &systems, uint32_t svId, const Transform &transform, const sf::Symbol &prefabName)
{
	uint32_t *pPrefabId = systems.entities.nameToPrefab.findValue(prefabName);
	if (!pPrefabId) return ~0u;
	uint32_t prefabId = *pPrefabId;

	Prefab &prefab = systems.entities.prefabs[prefabId];

	uint32_t entityId = systems.entities.addEntity(svId, transform, prefabId, prefab.entityIds.size);
	prefab.entityIds.push(entityId);

	addEntityComponents(systems, entityId, transform, prefab);

	return entityId;
}

static void removeEntities(Systems &systems, uint32_t svId)
{
	sf::SmallArray<uint32_t, 64> entityIds;

	{
		uint32_t entityId;
		sf::UintFind find = systems.entities.svToEntity.findAll(svId);
		while (find.next(entityId)) {
			entityIds.push(entityId);
		}
	}

	for (uint32_t entityId : entityIds) {
		systems.entities.removeEntity(systems, entityId);
	}
}

ClientState::ClientState()
{
	systems.area = AreaSystem::create();
	systems.model = ModelSystem::create();
}

void ClientState::applyEvent(const sv::Event &event)
{
	if (const auto *e = event.as<sv::LoadPrefabEvent>()) {
		systems.entities.addPrefab(e->prefab);
	} else if (const auto *e = event.as<sv::AddPropEvent>()) {
		Transform transform = getPropTransform(e->prop.transform);
		addEntity(systems, e->prop.id, transform, e->prop.prefabName);
	} else if (const auto *e = event.as<sv::ReplaceLocalPropEvent>()) {
		if (localClientId == e->clientId) {
			removeEntities(systems, e->localId);
		}
		Transform transform = getPropTransform(e->prop.transform);
		addEntity(systems, e->prop.id, transform, e->prop.prefabName);
	} else if (const auto *e = event.as<sv::MovePropEvent>()) {
		Transform transform = getPropTransform(e->transform);
		sf::UintFind find = systems.entities.svToEntity.findAll(e->propId);
		uint32_t entityId;
		while (find.next(entityId)) {
			systems.entities.updateTransform(systems, entityId, transform);
		}
	} else if (const auto *e = event.as<sv::RemovePropEvent>()) {
		removeEntities(systems, e->propId);
	} else if (const auto *e = event.as<sv::ReloadPrefabEvent>()) {
		if (uint32_t *pPrefabId = systems.entities.nameToPrefab.findValue(e->prefab.name)) {
			Prefab &prefab = systems.entities.prefabs[*pPrefabId];
			prefab.s = e->prefab;

			for (uint32_t entityId : prefab.entityIds) {
				Transform transform = systems.entities.entities[entityId].transform;
				systems.entities.removeComponents(systems, entityId);
				addEntityComponents(systems, entityId, transform, prefab);
			}
		} else {
			systems.entities.addPrefab(e->prefab);
		}
	}
}

void ClientState::editorPick(sf::Array<EntityHit> &hits, const sf::Ray &ray) const
{
	sf::FastRay fastRay { ray };

	sf::Array<Area> areas;
	systems.area->castRay(areas, Area::EditorPick, fastRay);

	for (Area &area : areas) {
		switch (area.group)
		{
		case AreaGroup::Model: systems.model->editorPick(hits, fastRay, area.userId); break;
		default:
			sf_failf("Unhandled EditorPick group: %u", area.group);
			break;
		}
	}
}

void ClientState::editorHighlight(uint32_t entityId, EditorHighlight type)
{
	Entity &entity = systems.entities.entities[entityId];
	for (const EntityComponent &ec : entity.components) {
		ec.system->editorHighlight(systems, ec, type);
	}
}

void ClientState::update(const FrameArgs &frameArgs)
{
	sf::Frustum frustum { frameArgs.worldToClip, sp::getClipNearW() };

	systems.model->updateLoadQueue(systems.area);

	systems.area->optimize();

	updateVisibility(systems.visibleAreas, systems.area, frustum);
}

void ClientState::renderMain(const RenderArgs &args)
{

	systems.model->renderMain(systems.visibleAreas, args);
}

}
