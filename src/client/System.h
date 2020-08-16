#pragma once

#include "sf/Matrix.h"
#include "sf/Box.h"
#include "sf/UintMap.h"
#include "sf/Array.h"
#include "sf/HashSet.h"
#include "sf/HashMap.h"
#include "sf/Symbol.h"
#include "sf/Frustum.h"

#include "server/ServerState.h"

#include "client/Transform.h"

namespace sv { struct Prefab; }
namespace sp { struct Canvas; }

namespace cl {

struct SystemsDesc
{
	uint64_t seed[4];
};

struct Systems;
struct EntityComponent;
struct FrameArgs;

enum class EditorHighlight
{
	Debug,
	Hint,
	Hover,
	Select,
};

struct TransformUpdate
{
	Transform previousTransform;
	Transform transform;
	sf::Mat34 entityToWorld;
};

struct System
{
	virtual ~System();
};

struct EntitySystem : System
{
	virtual void updateTransform(Systems &systems, uint32_t entityId, const EntityComponent &ec, const TransformUpdate &update) = 0;
	virtual bool prepareForRemove(Systems &systems, uint32_t entityId, const EntityComponent &ec, const FrameArgs &args);
	virtual void remove(Systems &systems, uint32_t entityId, const EntityComponent &ec) = 0;

	virtual void editorHighlight(Systems &systems, const EntityComponent &ec, EditorHighlight type);
};

struct EntityComponent
{
	EntitySystem *system;
	uint32_t userId;
	uint16_t flags;
	uint8_t subsystemIndex;
	uint8_t componentIndex;
};

struct Prefab
{
	sf::Box<sv::Prefab> svPrefab;
	sf::Array<uint32_t> entityIds;
};

struct Entity
{
	enum Flags
	{
		UpdateTransform = 0x1,
		PrepareForRemove = 0x2,
	};

	uint32_t svId = 0;
	Transform transform;
	uint32_t prefabId = ~0u;
	uint32_t indexInPrefab = 0;
	sf::SmallArray<EntityComponent, 4> components;
};

struct GuiArgs
{
	sp::Canvas *canvas = nullptr;
	sf::Vec2 resolution;
	float dt = 0.0f;
};

struct Entities
{
	sf::Array<Entity> entities;
	sf::Array<uint32_t> freeEntityIds;
	sf::UintMap svToEntity;

	sf::Array<Prefab> prefabs;
	sf::Array<uint32_t> freePrefabIds;
	sf::HashMap<sf::Symbol, uint32_t> nameToPrefab;

	sf::Array<uint32_t> removeQueue;

	uint32_t addPrefab(const sv::Prefab &svPrefab);
	void removePrefab(uint32_t prefabId);

	sf::Box<sv::Prefab> findPrefab(const sf::Symbol &name) const;

	uint32_t addEntityImp(uint32_t svId, const Transform &transform, uint32_t prefabId, uint32_t indexInPrefab);

	uint32_t addEntity(Systems &systems, uint32_t svId, const Transform &transform, const sf::Symbol &prefabName);
	void removeEntityInstant(Systems &systems, uint32_t entityId);
	void removeEntityQueued(uint32_t entityId);

	void updateTransform(Systems &systems, uint32_t entityId, const Transform &transform);

	void addComponents(Systems &systems, uint32_t entityId, const Transform &transform, const Prefab &prefab);
	void removeComponents(Systems &systems, uint32_t entityId);

	void addComponent(uint32_t entityId, EntitySystem *system, uint32_t userId, uint8_t subsystemIndex, uint8_t componentIndex, uint32_t flags);

	void updateQueuedRemoves(Systems &systems, const FrameArgs &frameArgs);
};

enum class AreaGroup : uint32_t
{
	DynamicModel,
	CharacterModel,
	TileChunkCulling,
	TileChunkActive,
	ParticleEffect,
	PointLight,
	Custom0,
};

struct Area
{
	enum Flags
	{
		Activate = 0x1,
		Visibility = 0x2,
		EditorPick = 0x4,
		Shadow = 0x8,
	};

	AreaGroup group;
	uint32_t flags;
	uint32_t userId;
};

struct AreaBounds
{
	Area area;
	sf::Bounds3 bounds;
};

struct VisibleAreas
{
	sf::Array<Area> areas;
	sf::Array<uint32_t> groups[(uint32_t)AreaGroup::Custom0];
	// TODO(?): sf::HashMap<uint32_t, sf::Array<uint32_t>> customGroups;

	sf_forceinline sf::Slice<const uint32_t> get(AreaGroup group) const {
		return groups[(uint32_t)group].slice();
	}
};

struct EntityHit
{
	uint32_t entityId = ~0u;
	float t = 0.0f;
};

struct AreaSystem;
struct LightSystem;
struct ModelSystem;
struct TileModelSystem;
struct CharacterModelSystem;
struct ParticleSystem;
struct GameSystem;

struct RenderArgs
{
	sf::Vec3 cameraPosition;
	sf::Mat34 worldToView;
	sf::Mat44 viewToClip;
	sf::Mat44 worldToClip;
	sf::Frustum frustum;
};

struct FrameArgs
{
	uint64_t frameIndex = 0;
	double gameTime = 0.0;
	float dt = 100.0f;

	RenderArgs mainRenderArgs;
};

struct Systems
{
	Entities entities;
	VisibleAreas activeAreas;
	VisibleAreas visibleAreas;
	VisibleAreas shadowAreas;
	sf::Box<AreaSystem> area;
	sf::Box<LightSystem> light;
	sf::Box<ModelSystem> model;
	sf::Box<TileModelSystem> tileModel;
	sf::Box<CharacterModelSystem> characterModel;
	sf::Box<ParticleSystem> particle;
	sf::Box<GameSystem> game;
	FrameArgs frameArgs;

	void init(const SystemsDesc &desc);

	void updateVisibility(VisibleAreas &areas, uint32_t areaFlags, const sf::Frustum &frustum);

	void renderShadows(const RenderArgs &renderArgs);
};

}
