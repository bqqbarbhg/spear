#pragma once

#include "sf/Vector.h"

namespace sv {

struct ConservativeLineRasterizer
{
	sf::Vec2i pos;
	sf::Vec2i end;
	sf::Vec2i unitDelta;
	sf::Vec2i absDelta;
	int32_t error;

	ConservativeLineRasterizer(const sf::Vec2i &begin, const sf::Vec2i &end);

	sf::Vec2i next();
};

}
