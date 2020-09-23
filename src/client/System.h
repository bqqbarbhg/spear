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

struct sapp_event;
namespace sv { struct Prefab; }
namespace sp { struct Canvas; }

namespace cl {

struct ClientPersist
{
	sf::Vec2 camera;
	float zoom = 0.0f;
};

struct SystemsDesc
{
	ClientPersist persist;
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

struct ComponentData
{
	const sv::Component *component;
	sf::Box<void> data;
};

struct Prefab
{
	sf::Box<sv::Prefab> svPrefab;
	sf::Array<uint32_t> entityIds;
	sf::Array<ComponentData> componentData;
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
	bool deleteQueued = false;
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

	void addPrefabComponents(Systems &systems, uint32_t prefabId);
	void removePrefabComponents(Systems &systems, uint32_t prefabId);

	uint32_t addPrefab(Systems &systems, const sv::Prefab &svPrefab);
	void removePrefab(Systems &systems, uint32_t prefabId);

	sf::Box<sv::Prefab> findPrefab(const sf::Symbol &name) const;

	uint32_t addEntityImp(uint32_t svId, const Transform &transform, uint32_t prefabId, uint32_t indexInPrefab);

	uint32_t addEntity(Systems &systems, uint32_t svId, const Transform &transform, const sf::Symbol &prefabName);
	void removeEntityInstant(Systems &systems, uint32_t entityId);
	void removeEntityQueued(uint32_t entityId);

	void updateTransform(Systems &systems, uint32_t entityId, const Transform &transform);

	void addComponents(Systems &systems, uint32_t entityId, const Transform &transform, const Prefab &prefab);
	void removeComponents(Systems &systems, uint32_t entityId);

	void addComponent(uint32_t entityId, EntitySystem *system, uint32_t userId, uint8_t subsystemIndex, uint8_t componentIndex, uint32_t flags);
	void removeComponent(uint32_t entityId, EntitySystem *system, uint32_t userId, uint8_t subsystemIndex);

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
	BlobShadow,
	TapArea,
	Custom0,
};

struct Area
{
	enum Flags
	{
		Activate = 0x1,
		Visibility = 0x2,
		Shadow = 0x4,
		EditorPick = 0x8,
		GamePick = 0x10,
		Envmap = 0x20,
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
	bool approximate = false;
};

struct BoneUpdates
{
	enum class Group : uint32_t
	{
		BlobShadow,

		Count,
	};

	struct Update
	{
		uint32_t userId;
		sf::Mat34 boneToWorld;
	};

	sf::Array<Update> updates[(uint32_t)Group::Count];

	void clear();
};

struct AreaSystem;
struct LightSystem;
struct EnvLightSystem;
struct ModelSystem;
struct TileModelSystem;
struct CharacterModelSystem;
struct ParticleSystem;
struct BlobShadowSystem;
struct VisFogSystem;
struct GameSystem;
struct TapAreaSystem;
struct BillboardSystem;
struct EffectSystem;
struct AudioSystem;

struct RenderArgs
{
	sf::Vec3 cameraPosition;
	sf::Mat34 worldToView;
	sf::Mat44 viewToClip;
	sf::Mat44 worldToClip;
	sf::Frustum frustum;
	sf::Vec2i targetResolution;
	sf::Vec2i renderResolution;
	bool flipCulling = false;
};

struct FrameArgs
{
	uint64_t frameIndex = 0;
	double gameTime = 0.0;
	float dt = 100.0f;
	sf::Slice<const sapp_event> events;
	sf::Vec2i windowResolution;
	sf::Vec2 guiResolution;
	bool editorOpen = false;

	RenderArgs mainRenderArgs;
};

struct Systems
{
	Entities entities;
	VisibleAreas activeAreas;
	VisibleAreas visibleAreas;
	VisibleAreas shadowAreas;
	VisibleAreas envmapAreas;
	BoneUpdates boneUpdates;
	sf::Box<AreaSystem> area;
	sf::Box<LightSystem> light;
	sf::Box<EnvLightSystem> envLight;
	sf::Box<ModelSystem> model;
	sf::Box<TileModelSystem> tileModel;
	sf::Box<CharacterModelSystem> characterModel;
	sf::Box<ParticleSystem> particle;
	sf::Box<BlobShadowSystem> blobShadow;
	sf::Box<VisFogSystem> visFog;
	sf::Box<GameSystem> game;
	sf::Box<TapAreaSystem> tapArea;
	sf::Box<BillboardSystem> billboard;
	sf::Box<EffectSystem> effect;
	sf::Box<AudioSystem> audio;
	FrameArgs frameArgs;

	void init(const SystemsDesc &desc);

	void updateVisibility(VisibleAreas &areas, uint32_t areaFlags, const sf::Frustum &frustum);

	void renderShadows(const RenderArgs &renderArgs);
	void renderEnvmapGBuffer(const RenderArgs &renderArgs);
};

}
