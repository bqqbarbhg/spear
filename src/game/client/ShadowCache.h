#pragma once

#include "sp/Renderer.h"
#include "sf/HashMap.h"

namespace cl {

struct State;
struct PointLight;

struct ShadowTile
{
	uint32_t x, y;
	uint32_t lightIndex;
};

struct ShadowCache
{
	uint32_t cacheTileExtent = 64;
	uint32_t cacheTileSlices = 8;
	uint32_t cacheNumTilesX = 8;
	uint32_t cacheNumTilesY = 8;

	uint32_t depthRenderExtent = 128;

	sp::Texture shadowCache;

	sp::RenderTarget shadowCacheRender;
	sp::RenderPass shadowCachePass;

	sp::Texture depthRenderCube;
	sp::RenderTarget depthRenderDepth;
	sp::RenderPass depthRenderPass[6];

	sf::HashMap<uint32_t, ShadowTile> shadowTiles;

	void updatePointLight(State &cs, PointLight &light);

	void recreateTargets();
};

}
