#pragma once

#include "sp/Renderer.h"
#include "sf/HashMap.h"

#if SF_OS_WASM
	#define CL_SHADOWCACHE_USE_ARRAY 1
	#define CL_SHADOWCACHE_TEX TEX_shadowGridArray
#else
	#define CL_SHADOWCACHE_USE_ARRAY 0 
	#define CL_SHADOWCACHE_TEX TEX_shadowGrid3D
#endif

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
	uint32_t cacheTileExtent = 128;
	uint32_t cacheTileSlices = 8;
	uint32_t cacheNumTilesX = 8;
	uint32_t cacheNumTilesY = 8;

	uint32_t depthRenderExtent = 512;

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
