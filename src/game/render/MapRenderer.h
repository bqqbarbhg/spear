#pragma once

#include "sf/Symbol.h"
#include "sf/Matrix.h"
#include "game/Entity.h"

struct MapModel
{
	sf::Symbol modelName;
	sf::Mat34 transform;
};

struct MapRenderer
{
	MapRenderer();
	~MapRenderer();

	void addMapModel(Entity e, const MapModel &model);
	void removeMapModel(Entity e);

	void update();
	void testRenderLight();
	void render();

	struct Data;
	Data *data;
};
