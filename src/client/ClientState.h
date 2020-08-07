#pragma once

#include "server/ServerState.h"
#include "client/EntityState.h"
#include "sf/Array.h"
#include "sf/Matrix.h"
#include "sf/Frustum.h"
#include "sf/HashSet.h"

namespace cl {

struct RenderShadowArgs
{
	sf::Vec3 cameraPosition;
	sf::Frustum frustum;
	sf::Mat44 worldToClip;
};

struct Entity
{
	uint32_t id;
	EntityState state;
	uint32_t movedIndex = ~0u;
};

struct LightState;
struct AreaState;
struct ClientState
{
	sf::Box<LightState> lightState;
	sf::Box<AreaState> areaState;

	sf::ImplicitHashMap<Entity, sv::KeyId> entities;
	sf::HashSet<uint32_t> visibleEntities;

	void renderShadows(const RenderShadowArgs &args);
};

#if 0

struct Component
{
	#if SF_DEBUG
		virtual void debugForceVtable() { }
	#endif

	using Type = sv::Component::Type;

	Type type;

	Component() { }
	Component(Type type) : type(type) { }

	template <typename T> T *as() { return type == T::ComponentType ? (T*)this : nullptr; }
	template <typename T> const T *as() const { return type == T::ComponentType ? (T*)this : nullptr; }
};

template <Component::Type SelfType>
struct ComponentBase : Component
{
	static constexpr Type ComponentType = SelfType;
	ComponentBase() : Component(SelfType) { }
};

struct ModelComponent : ComponentBase<sv::Component::Model>
{
	sf::Box<sv::ModelComponent> s;
	sp::ModelRef model;
};

struct CharacterModelComponent : ComponentBase<sv::Component::CharacterModel>
{
	sf::Box<sv::CharacterModelComponent> s;
	sp::ModelRef model;
};

struct CardComponent : ComponentBase<sv::Component::Card>
{
	sf::Box<sv::CardComponent> s;
	sp::SpriteRef sprite;
	uint32_t cooldownLeft = 0;
	float cooldownAnim = 0.0f;
};

struct Entity
{
	uint32_t id;
	sf::Sphere bounds;
	sf::Mat34 transform;
	sf::Array<sf::Box<Component>> components;
};

struct LightState;
struct ClientState
{
	sf::Box<LightState> lights;

	sf::ImplicitHashMap<Entity, sv::KeyId> entities;

	void applyEvent(const sv::Event &event, bool simulated);
};


enum class IdType {
	Null = (IdType)sv::IdType::Null,
	Prop = (IdType)sv::IdType::Prop,
	Character = (IdType)sv::IdType::Character,
	Card = (IdType)sv::IdType::Card,
	Status = (IdType)sv::IdType::Status,
	ClientStart,
	AttachedItem,
	Model,
	CharacterModel,
	PointLight,
	ParticleSystem,
	Projectile,
};

static_assert((uint32_t)IdType::ClientStart == (uint32_t)sv::IdType::ClientStart, "IdType out of sync");

struct Prop
{
};

struct Character
{
};

struct Status
{
	sf::SmallArray<uint32_t, 4> effects;
};

struct Card
{
	sf::Symbol name;
	sf::Symbol description;
	sp::SpriteRef sprite;
	uint32_t cooldownLeft = 0;
	float cooldownAnim = 0.0f;
};

struct AttachedItem
{
	uint32_t id;
	uint32_t entityId;
};

struct Transform
{
	uint32_t id;
	uint32_t parentId;
	sf::Mat34 worldTransform;
	sf::HashMap<sf::Symbol, Transform> bones;
	sf::Array<uint32_t> children;
};

struct PointLight
{
	uint32_t id;
	uint32_t entityId;
	uint32_t shadowIndex;
};

struct ParticleSystem
{
};

struct Projectile
{
};

struct Prefab
{
	sv::Prefab s;
};

struct ClientState
{
	sf::ImplicitHashMap<Prefab, sv::KeyName> prefabs;
	sf::ImplicitHashMap<Prop, sv::KeyId> props;
	sf::ImplicitHashMap<Character, sv::KeyId> characters;
	sf::ImplicitHashMap<Card, sv::KeyId> cards;
	sf::ImplicitHashMap<Status, sv::KeyId> statuses;
	sf::ImplicitHashMap<AttachedItem, sv::KeyId> attachedItems;
	sf::ImplicitHashMap<Transform, sv::KeyId> transforms;
	sf::ImplicitHashMap<PointLight, sv::KeyId> pointLights;
	sf::ImplicitHashMap<ParticleSystem, sv::KeyId> particleSystems;
	sf::ImplicitHashMap<Projectile, sv::KeyId> projectiles;

	void spawnEffect(const sf::Symbol &prefab, uint32_t parent);

	void applyEvent(const sv::Event &event, bool simulated);
};

#endif

}
