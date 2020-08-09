#pragma once

#include "sf/Vector.h"
#include "sf/Quaternion.h"
#include "sf/Matrix.h"

namespace cl {

struct Transform
{
	sf::Vec3 position;
	sf::Quat rotation;
	float scale = 1.0f;

	sf::Mat34 asMatrix() const {
		return sf::mat::world(position, rotation, scale);
	}
};

}
