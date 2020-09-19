#pragma once

#include "sf/Vector.h"
#include "sf/Matrix.h"

#define SpShader_DebugEnvSphere 0
#define SpShader_DynamicMesh 1
#define SpShader_EnvmapLighting 2
#define SpShader_TestDepthPrepass 3
#define SpShader_TestMesh 4
#define SpShader_TestSkin 5
#define SpShaderDataSize 157918

#define SP_SHADOWGRID_USE_ARRAY 0
#define SP_NORMALMAP_REMAP 1
#define SP_DEBUG_MODE 2
#define SP_DIRECT_ENV_LIGHT 3
#define SP_NUM_PERMUTATIONS 4

struct UBO_DebugEnvSphereVertex {
	static const constexpr uint32_t UboIndex = 1;

	sf::Mat44 worldToClip;
	sf::Vec4 sphereGridMad;
	sf::Vec2i sphereGridSize;
	uint32_t _sp_pad0[2];
	sf::Vec4 layerHeights;
	int32_t numLayers;
	float sphereRadius;
};

struct UBO_DebugEnvSpherePixel {
	static const constexpr uint32_t UboIndex = 2;

	sf::Vec4 diffuseEnvmapMad;
	sf::Vec3 cameraPosition;
	float specular;
};

struct UBO_DynamicTransform {
	static const constexpr uint32_t UboIndex = 3;

	sf::Mat44 modelToWorld;
	sf::Mat44 worldToClip;
};

struct UBO_Pixel {
	static const constexpr uint32_t UboIndex = 4;

	float numLightsF;
	uint32_t _sp_pad0[3];
	sf::Vec3 cameraPosition;
	uint32_t _sp_pad1[1];
	sf::Vec4 diffuseEnvmapMad;
	sf::Vec4 pointLightData[64];
};

struct UBO_EnvmapVertex {
	static const constexpr uint32_t UboIndex = 5;

	float flipX;
	float flipY;
	float pad_2;
	float pad_3;
};

struct UBO_EnvmapPixel {
	static const constexpr uint32_t UboIndex = 6;

	sf::Mat44 clipToWorld;
	float numLightsF;
	float depthToDistance;
	uint32_t _sp_pad0[2];
	sf::Vec4 uvMad;
	sf::Vec3 rayDir;
	uint32_t _sp_pad1[1];
	sf::Vec4 diffuseEnvmapMad;
	sf::Vec4 pointLightData[512];
};

struct UBO_Transform {
	static const constexpr uint32_t UboIndex = 7;

	sf::Mat44 worldToClip;
};

struct UBO_SkinTransform {
	static const constexpr uint32_t UboIndex = 8;

	sf::Mat44 worldToClip;
};

struct UBO_Bones {
	static const constexpr uint32_t UboIndex = 9;

	sf::Vec4 bones[192];
};

struct UBO_SkinPixel {
	static const constexpr uint32_t UboIndex = 10;

	float numLightsF;
	uint32_t _sp_pad0[3];
	sf::Vec3 cameraPosition;
	uint32_t _sp_pad1[1];
	sf::Vec3 highlightColor;
	uint32_t _sp_pad2[1];
	sf::Vec2 highlightMad;
	uint32_t _sp_pad3[2];
	sf::Vec4 diffuseEnvmapMad;
	sf::Vec4 pointLightData[64];
};

#define TEX_envmap 1
#define TEX_diffuseEnvmapAtlas 2
#define TEX_shadowGrid3D 3
#define TEX_albedoTexture 4
#define TEX_normalTexture 5
#define TEX_maskTexture 6
#define TEX_shadowGridArray 7
#define TEX_gbuffer0 8
#define TEX_gbuffer1 9
#define TEX_albedoAtlas 10
#define TEX_normalAtlas 11
#define TEX_maskAtlas 12

struct SpShaderInfo;
struct SpPermutationInfo;
struct SpUniformBlockInfo;
struct SpSamplerInfo;
struct SpAttribInfo;
extern const SpShaderInfo spShaders[6];
extern const SpPermutationInfo spPermutations[28];
extern const SpUniformBlockInfo spUniformBlock[11];
extern const SpSamplerInfo spSamplers[13];
extern const SpAttribInfo spAttribs[12];
extern const char spShaderData[4339];