#pragma once

#include "server/ServerState.h"
#include "client/EntityState.h"
#include "sf/Array.h"
#include "sf/Matrix.h"
#include "sf/Frustum.h"
#include "sf/HashSet.h"

namespace cl {

struct RenderArgs
{
	sf::Vec3 cameraPosition;
	sf::Frustum frustum;
	sf::Mat44 worldToClip;
};

struct Entity
{
	uint32_t id;
	sf::Symbol prefabName;
	EntityState state;
};

struct Prefab
{
	sv::Prefab s;
};

struct PrefabKey { sf::Symbol& operator()(Prefab &t) { return t.s.name; } };

struct LightState;
struct AreaState;
struct MeshState;
struct SpatialNode;
struct ClientState
{
	sf::ImplicitHashMap<Prefab, PrefabKey> prefabs;

	sf::Box<AreaState> areaState;
	sf::Box<LightState> lightState;
	sf::Box<MeshState> meshState;

	sf::ImplicitHashMap<Entity, sv::KeyId> entities;
	sf::HashSet<uint32_t> visibleEntities;
	sf::Array<const SpatialNode*> visibleNodes;

	ClientState();

	void addEntity(uint32_t entityId, const sf::Symbol &prefabName, const sf::Mat34 &transform);

	void applyEvent(const sv::Event &event);

	void updateAssetLoading();

	void updateVisibility(const sf::Frustum &frustum);

	void renderShadows(const RenderArgs &args);
	void renderMain(const RenderArgs &args);
};

}
