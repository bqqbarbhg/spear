#pragma once

#include "game/server/GameState.h"
#include "game/server/Event.h"
#include "game/client/AssetInfo.h"
#include "game/client/MapMesh.h"
#include "game/client/ShadowCache.h"
#include "sf/Frustum.h"
#include "sf/Geometry.h"
#include "sf/HashSet.h"
#include "sp/Sprite.h"

namespace cl {

struct MapChunk
{
	static constexpr const uint32_t SizeLog2 = 4;
	static constexpr const uint32_t Size = 1u << SizeLog2;

	static sf_forceinline void getChunkAndTile(sf::Vec2i &chunkI, sf::Vec2i &tileI, const sf::Vec2i &pos)
	{
		chunkI = { pos.x >> (int32_t)MapChunk::SizeLog2, pos.y >> (int32_t)MapChunk::SizeLog2 };
		tileI = { pos.x & ((int32_t)MapChunk::Size - 1), pos.y & ((int32_t)MapChunk::Size - 1) };
	}

	static sf_forceinline sf::Vec2i getChunk(const sf::Vec2i &pos)
	{
		return { pos.x >> (int32_t)MapChunk::SizeLog2, pos.y >> (int32_t)MapChunk::SizeLog2 };
	}

	sf::HashSet<sv::InstanceId> meshInstances;
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

struct PointLight
{
	sf::Vec3 position;
	sf::Vec3 color;
	float radius;
	uint32_t shadowIndex = ~0u;

	sf::Vec3 shadowMul;
	sf::Vec3 shadowBias;

	sv::InstanceId objectId;
};

struct RenderShadowArgs
{
	sf::Vec3 cameraPosition;
	sf::Frustum frustum;
	sf::Mat44 worldToClip;
};

struct Object
{
	sv::GameObject sv;
	sf::Array<MapMesh> mapMeshes;
	sf::Array<PointLight> pointLights;
	sf::HashSet<sv::InstanceId> instances;
	bool hasValidBounds = false;
	sf::Sphere bounds;
};

struct Instance
{
	sv::InstancedObject sv;
};

struct State
{
	sf::HashMap<sf::Vec2i, MapChunk> chunks;
	sf::Array<sf::Vec2i> dirtyChunks;
	sf::Array<PointLight> pointLights;
	sf::HashMap<sv::ObjectId, Object> objects;
	sf::HashMap<sv::InstanceId, Instance> instances;
	sf::HashMap<sv::InstanceId, sf::SmallArray<uint32_t, 1>> pointLightMapping;
	ShadowCache shadowCache;

	sf::Sphere getObjectBounds(Object &object);
	sf::Vec3 getInstancePosition(const sv::InstancedObject &object);
	sf::Mat34 getInstanceTransform(const sv::InstancedObject &object);
	void getObjectBounds(const Object &type, const sf::Mat34 &transform, sf::Array<sf::Mat34> &bounds);

	sv::InstanceId pickInstance(float &outT, const sf::Ray &ray);

	void reset(sv::State *svState);

	void applyEvent(sv::Event *event);

	void renderShadows(const RenderShadowArgs &args);

	void updateMapChunks(sv::State &svState);

	void recreateTargets();
	void assetsReloaded();
};

}
