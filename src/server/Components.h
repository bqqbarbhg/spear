#pragma once

#include "server/Entity.h"
#include "sf/Vector.h"

namespace sv {

struct ModelComponent : ComponentBase<Component::Model>
{
	sf::Symbol model;
	sf::Symbol shadowModel;
	sf::Symbol material;
	sf::Vec3 position;
	sf::Vec3 rotation;
	float scale = 1.0f;
	sf::Vec3 stretch = sf::Vec3(1.0f);
	bool castShadows = true;
};

struct PointLightComponent : ComponentBase<Component::PointLight>
{
	sf::Vec3 color = sf::Vec3(1.0f);
	float intensity = 1.0f;
	float radius = 1.0f;
	sf::Vec3 position;
};

struct TileAreaComponent : ComponentBase<Component::TileArea>
{
	sf::Vec2i offset;
	sf::Vec2i size;
	bool floor = false;
	bool wall = false;
};

}
