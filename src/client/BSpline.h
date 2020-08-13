#pragma once

#include "sf/Vector.h"

namespace cl {

void discretizeBSplineY(sf::Slice<float> yValues, sf::Slice<const sf::Vec2> points, float xMin, float xMax);

}
