#pragma once

#include "server/ServerState.h"
#include "sf/Array.h"
#include "sf/Matrix.h"
#include "sf/Frustum.h"
#include "sf/HashSet.h"
#include "sf/Quaternion.h"
#include "client/VisualTransform.h"

namespace cl {

struct RenderArgs
{
	sf::Vec3 cameraPosition;
	sf::Frustum frustum;
	sf::Mat44 worldToClip;
};

struct Entity
{
	enum Mask
	{
		Mesh = 0x1,
		Area = 0x2,
		Light = 0x4,
	};

	uint32_t id;
	sf::Symbol prefabName;
	VisualTransform transform;
	uint32_t mask = 0;
};

struct Prefab
{
	sv::Prefab s;
};

struct EntityInterpolation
{
	uint32_t entityId;
	VisualHermite spline;
	float t = 0.0f;
	float speed = 1.0f;
};

struct EntityInterpolationOpts
{
	float duration = 0.0f;
	float velocityBegin = 0.0f;
	float velocityEnd = 0.0f;
	float velocityInherit = 1.0f;
};

struct PrefabKey { sf::Symbol& operator()(Prefab &t) { return t.s.name; } };
struct KeyEntityId { template <typename T> decltype(T::entityId)& operator()(T &t) { return t.entityId; } };

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

	sf::ImplicitHashMap<EntityInterpolation, KeyEntityId> entityInterpolations;

	ClientState();

	void registerEntity(uint32_t entityId, Entity::Mask mask);

	void addEntity(uint32_t entityId, const sf::Symbol &prefabName, const VisualTransform &transform);

	void setEntityTransform(uint32_t entityId, const VisualTransform &transform);

	void addEntityInterpolation(uint32_t entityId, const VisualTransform &dst, const EntityInterpolationOpts &opts);
	void updateEntityInterpolations(float dt);

	void applyEvent(const sv::Event &event);

	void updateAssetLoading();

	void updateVisibility(const sf::Frustum &frustum);

	void renderShadows(const RenderArgs &args);
	void renderMain(const RenderArgs &args);
};

}
