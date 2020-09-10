#pragma once

#include "server/ServerState.h"
#include "ext/sokol/sokol_defs.h"

#include "client/System.h"

namespace sf { struct FastRay; }

namespace cl {

#if SF_OS_WASM
	#define CL_SHADOWCACHE_USE_ARRAY 1
	#define CL_SHADOWCACHE_TEX TEX_shadowGridArray
#else
	#define CL_SHADOWCACHE_USE_ARRAY 0 
	#define CL_SHADOWCACHE_TEX TEX_shadowGrid3D
#endif

struct PointLight
{
	sf::Vec3 position;
	float radius;
	sf::Vec3 color;

	sf::Vec3 shadowMul;
	sf::Vec3 shadowBias;

	void writeShader(sf::Vec4 *&dst)
	{
		dst[0].x = position.x;
		dst[0].y = position.y;
		dst[0].z = position.z;
		dst[0].w = radius;
		dst[1].x = color.x;
		dst[1].y = color.y;
		dst[1].z = color.z;
		dst[1].w = 0.0f;
		dst[2].x = shadowMul.x;
		dst[2].y = shadowMul.y;
		dst[2].z = shadowMul.z;
		dst[2].w = 0.0f;
		dst[3].x = shadowBias.x;
		dst[3].y = shadowBias.y;
		dst[3].z = shadowBias.z;
		dst[3].w = 0.0f;
		dst += 4;
	}
};

struct LightSystem : EntitySystem
{
	static sf::Box<LightSystem> create();

	virtual void addPointLight(Systems &systems, uint32_t entityId, uint8_t componentIndex, const sv::PointLightComponent &c, const Transform &transform) = 0;

	virtual void updateLightFade(const FrameArgs &frameArgs) = 0;

	virtual void queryVisiblePointLights(const VisibleAreas &visibleAreas, sf::Array<PointLight> &pointLights, const sf::Bounds3 &bounds) const = 0;

	virtual void renderShadowMaps(Systems &systems, const VisibleAreas &visibleAreas, uint64_t frameIndex) = 0;

	virtual void setIblEnabled(bool enabled) = 0;

	virtual sg_image getShadowTexture() const = 0;
	virtual sg_image getEnvmapTexture(const sf::Bounds3 &bounds) const = 0;
};

}
