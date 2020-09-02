#include "LineRasterizer.h"

namespace sv {

ConservativeLineRasterizer::ConservativeLineRasterizer(const sf::Vec2i &begin, const sf::Vec2i &end)
{
	this->pos = begin;
	this->end = end;
	sf::Vec2i delta = end - begin;
	unitDelta.x = delta.x > 0 ? +1 : -1;
	unitDelta.y = delta.y > 0 ? +1 : -1;
	absDelta = unitDelta * delta;
	error = absDelta.x - absDelta.y;
}

sf::Vec2i ConservativeLineRasterizer::next()
{
	sf::Vec2i prevPos = pos;

	if (error > 0) {
		pos.x += unitDelta.x;
		error -= absDelta.y * 2;
	} else if (error < 0) {
		pos.y += unitDelta.y;
		error += absDelta.x * 2;
	} else {
		pos += unitDelta;
	}

	return prevPos;
}

}
