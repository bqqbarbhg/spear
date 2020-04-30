#pragma once

#include "sf/Vector.h"
#include "sf/Array.h"

#include "Entity.h"

struct Map
{
	Map();
	~Map();

	sf::Vec2i getTile(Entity e);
	void setTile(Entity e, const sf::Vec2i &tile);
	void getEntitiesInTile(const sf::Vec2i &tile, sf::Array<Entity> &entities);

	struct Data;
	Data *data;
};
