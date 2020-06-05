#pragma once

#include "game/server/GameState.h"
#include "game/server/Event.h"
#include "game/client/AssetInfo.h"
#include "game/client/MapMesh.h"
#include "game/client/ShadowCache.h"
#include "sf/Frustum.h"
#include "sf/HashSet.h"
#include "sp/Sprite.h"

namespace cl {

struct TileType
{
	TileInfoRef floor;
	TileInfoRef tile;
};

struct MapChunk
{
	sf::HashSet<uint32_t> meshObjects;
	sf::Array<MapMesh> meshes;
	MapChunkGeometry geometry;
	bool meshesDirty = false;
	bool dirty = false;
};

struct Card
{
	static constexpr const float Aspect = 1.0f / 1.5f;

	sv::Card svCard;
	sp::SpriteRef imageSprite;
};

struct Entity
{
	#if SF_DEBUG
		virtual void debugForceVtable() { }
	#endif

	using Type = sv::Entity::Type;

	sv::Entity::Type type;
	sf::Box<sv::Entity> svEntity;
	sf::Vec2 position;

	Entity() { }
	Entity(Type type, sf::Box<sv::Entity> svEntity) : type(type), svEntity(std::move(svEntity)) { }

	template <typename T> T *as() { return type == T::EntityType ? (T*)this : nullptr; }
	template <typename T> const T *as() const { return type == T::EntityType ? (T*)this : nullptr; }
};

template <Entity::Type SelfType>
struct EntityBase : Entity
{
	static constexpr Type EntityType = SelfType;
	EntityBase(sf::Box<sv::Entity> svEntity) : Entity(SelfType, svEntity) { }
};

struct Character : EntityBase<sv::Entity::Character>
{
	Character(sf::Box<sv::Entity> svEntity) : EntityBase(std::move(svEntity)) { }

	sf::Array<Card> cards;

	ModelInfoRef model;
	sf::Array<sv::Waypoint> waypoints;
};

struct PointLight
{
	sf::Vec3 position;
	sf::Vec3 color;
	float radius;
	uint32_t shadowIndex = ~0u;

	sf::Vec3 shadowMul;
	sf::Vec3 shadowBias;

	uint32_t objectId;
};

struct RenderShadowArgs
{
	sf::Vec3 cameraPosition;
	sf::Frustum frustum;
	sf::Mat44 worldToClip;
};

struct Component
{
	using Type = sv::Component::Type;

	#if SF_DEBUG
		virtual void debugForceVtable() { }
	#endif

	Type type;

	Component() { }
	Component(Type type) : type(type) { }

	template <typename T> T *as() { return type == T::ComponentType ? (T*)this : nullptr; }
	template <typename T> const T *as() const { return type == T::ComponentType ? (T*)this : nullptr; }
};

struct ObjectType
{
	sv::GameObject svType;
	sf::Array<MapMesh> mapMeshes;
	sf::Array<PointLight> pointLights;
	sf::HashSet<uint32_t> objects;
};

struct Object
{
	sv::Object svObject;
};

struct State
{
	sf::Array<TileType> tileTypes;
	sf::Array<sf::Box<Entity>> entities;
	sf::HashMap<sf::Vec2i, MapChunk> chunks;
	sf::Array<sf::Vec2i> dirtyChunks;
	sf::Array<PointLight> pointLights;
	sf::Array<ObjectType> objectTypes;
	sf::HashMap<uint32_t, Object> objects;
	sf::HashMap<uint32_t, sf::SmallArray<uint32_t, 1>> pointLightMapping;
	ShadowCache shadowCache;

	sf::Vec3 getObjectPosition(const sv::Object &object);

	void reset(sv::State *svState);

	void applyEvent(sv::Event *event);

	void renderShadows(const RenderShadowArgs &args);

	void updateMapChunks(sv::State &svState);

	void recreateTargets();
	void assetsReloaded();
};

}
