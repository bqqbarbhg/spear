#pragma once

#include "sf/Geometry.h"
#include "sp/Asset.h"

#include "ext/sokol/sokol_gfx.h"

namespace sf { struct Random; }
namespace sf { struct Frustum; }

namespace cl {

struct ParticleTexture : sp::Asset
{
	static sp::AssetType SelfType;
	using PropType = sp::NoAssetProps;

	sg_image image;

	static sg_pixel_format pixelFormat;
	static sg_image defaultImage;

	// Lifecycle
	static void globalInit();
	static void globalCleanup();
};

using ParticleTextureRef = sp::Ref<ParticleTexture>;

struct RandomVec3
{
	sf::Vec3 origin;
	sf::Vec3 boxExtent;
	float sphereMinRadius = 0.0f;
	float sphereMaxRadius = 0.0f;

	sf::Vec3 sample(sf::Random &rng) const;
};

struct ParticleFloat
{
	float base = 1.0f;
	float variance = 0.0f;
	float fadeIn = 0.0f;
	float fadeOut = 0.0f;

	sf::Vec4 packShader() const;
};

struct ParticleSystem
{
	RandomVec3 spawnPosition;
	RandomVec3 spawnVelocity;

	sf::Vec3 gravity;
	float drag = 0.0f;

	float spawnTime = 1.0f;
	float spawnTimeVariance = 0.0f;

	float cullPadding = 1.0f;

	float timeStep = 0.1f;

	ParticleTextureRef texture;
	sf::Vec2i frameCount = sf::Vec2i(1, 1);
	float frameRate = 0.0f;

	ParticleFloat scaleAnim;
	ParticleFloat alphaAnim;
	float angle = 0.0f;
	float angleVariance = 0.0f;
	float spin = 0.0f;
	float spinVariance = 0.0f;

	void update(float dt, sf::Random &rng);
	void render(const sf::Mat34 &worldToView, const sf::Mat44 &viewToClip, const sf::Frustum &frustum);

	// Lifecycle

	static ParticleSystem *create();
	static void free(ParticleSystem *s);

	static void globalInit();
	static void globalCleanup();
	static void globalUpdate();
};

}
