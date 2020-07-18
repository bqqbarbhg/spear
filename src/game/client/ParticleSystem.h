#pragma once

#include "sf/Geometry.h"

namespace sf { struct Random; }
namespace sf { struct Frustum; }

namespace cl {

struct RandomVec3
{
	sf::Vec3 origin;
	sf::Vec3 boxExtent;
	float sphereMinRadius = 0.0f;
	float sphereMaxRadius = 0.0f;

	sf::Vec3 sample(sf::Random &rng);
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
