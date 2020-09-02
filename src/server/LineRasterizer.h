#pragma once

#include "sf/Vector.h"

namespace sv {

struct ConservativeLineRasterizer
{
	sf::Vec2i pos;
	sf::Vec2i end;
	sf::Vec2i step;
	sf::Vec2i num;
	sf::Vec2i t;

	ConservativeLineRasterizer(const sf::Vec2i &begin, const sf::Vec2i &end);

	sf::Vec2i next();
};

}
