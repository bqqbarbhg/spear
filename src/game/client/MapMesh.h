#pragma once

#include "sp/Renderer.h"
#include "sp/Model.h"
#include "sf/Geometry.h"
#include "game/server/GameState.h"
#include "TileMaterial.h"

namespace cl {

struct MapVertex
{
	sf::Vec3 position;
	sf::Vec3 normal;
	sf::Vec4 tangent;
	sf::Vec2 uv;
};

struct MapMesh
{
	sp::ModelRef model;
	sp::ModelRef shadowModel;
	cl::TileMaterialRef material;
	sf::Mat34 transform;
};

struct MapGeometry
{
	sf::Bounds3 bounds;
	sp::Buffer vertexBuffer;
	sp::Buffer indexBuffer;
	uint32_t numInidces = 0;
	bool largeIndices = false;

	void reset();
};

struct MapChunkGeometry
{
	MapGeometry main;
	MapGeometry shadow;

	void reset();
	bool build(sf::Slice<MapMesh> meshes, const sf::Vec2i &chunkPos);
};

}
