#include "ClientState.h"

#include "AreaState.h"
#include "LightState.h"
#include "MeshState.h"

namespace cl {

static sf::Mat34 getPropTransform(const sv::Prop &prop)
{
	sf::Vec3 pos = sf::Vec3((float)prop.tile.x, 0.0f, (float)prop.tile.y) + prop.offset;
	return sf::mat::translate(pos);
}

ClientState::ClientState()
{
	areaState = AreaState::create();
	lightState = LightState::create();
	meshState = MeshState::create();
}

void ClientState::addEntity(uint32_t entityId, const sf::Symbol &prefabName, const sf::Mat34 &transform)
{
	Entity &entity = entities[entityId];
	entity.id = entityId;
	entity.prefabName = prefabName;
	entity.state.transform = transform;

	Prefab *prefab = prefabs.find(prefabName);
	if (!prefab) return;

	for (const sf::Box<sv::Component> &comp : prefab->s.components) {

		if (const auto *c = comp->as<sv::ModelComponent>()) {
			meshState->addMesh(entityId, comp.cast<sv::ModelComponent>());
		}

	}
}

void ClientState::applyEvent(const sv::Event &event)
{
	if (const auto *e = event.as<sv::LoadPrefabEvent>()) {
		Prefab &prefab = prefabs[e->prefab.name];
		prefab.s = e->prefab;
	} else if (const auto *e = event.as<sv::AddPropEvent>()) {
		sf::Mat34 transform = getPropTransform(e->prop);
		addEntity(e->prop.id, e->prop.prefabName, transform);
	}
}

void ClientState::updateAssetLoading()
{
	meshState->updatePendingMeshes();
}

void ClientState::updateVisibility(const sf::Frustum &frustum)
{
	areaState->optimizeSpatialNodes();

	visibleNodes.clear();
	areaState->querySpatialNodesFrustum(visibleNodes, frustum);
	areaState->updateMainVisibility(visibleNodes, frustum);

	lightState->updateVisibility();
}

void ClientState::renderShadows(const RenderArgs &args)
{
}

void ClientState::renderMain(const RenderArgs &args)
{
	meshState->renderMeshses(args);
}

}
