#pragma once

#include "sp/Renderer.h"
#include "sp/Model.h"
#include "sf/Geometry.h"
#include "game/server/GameState.h"

namespace cl {

struct MapMesh
{
	sp::ModelRef model;
	sp::ModelRef shadowModel;
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
	void build(sf::Slice<MapMesh> meshes, const sf::Vec2i &chunkPos);
};

}
