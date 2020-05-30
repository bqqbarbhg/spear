#pragma once

#include "game/server/GameState.h"
#include "game/server/Event.h"
#include "game/client/AssetInfo.h"
#include "game/client/MapMesh.h"
#include "game/client/ShadowCache.h"
#include "sf/Frustum.h"

namespace cl {

struct TileType
{
	TileInfoRef floor;
	TileInfoRef tile;
};

struct MapChunk
{
	sf::Array<MapMesh> meshes;
	MapChunkGeometry geometry;
	bool meshesDirty = false;
	bool dirty = false;
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

	ModelInfoRef model;
	sf::Array<sv::Waypoint> waypoints;
};

struct PointLight
{
	sf::Vec3 position;
	sf::Vec3 color;
	float radius;
	uint32_t shadowIndex;

	sf::Vec3 shadowMul;
	sf::Vec3 shadowBias;
};

struct RenderShadowArgs
{
	sf::Vec3 cameraPosition;
	sf::Frustum frustum;
	sf::Mat44 worldToClip;
};

struct State
{
	sf::Array<TileType> tileTypes;
	sf::Array<sf::Box<Entity>> entities;
	sf::HashMap<sf::Vec2i, MapChunk> chunks;
	sf::Array<sf::Vec2i> dirtyChunks;
	sf::Array<PointLight> pointLights;
	ShadowCache shadowCache;

	void reset(sv::State *svState);

	void applyEvent(sv::Event *event);

	void renderShadows(const RenderShadowArgs &args);

	void updateMapChunks(sv::State &svState);

	void recreateTargets();
	void assetsReloaded();
};

}
