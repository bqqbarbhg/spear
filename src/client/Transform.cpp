#include "Transform.h"

namespace cl {

sf::Vec3 Transform::transformPoint(const sf::Vec3 &point) const
{
	return sf::rotate(rotation, point * scale) + position;
}

}
