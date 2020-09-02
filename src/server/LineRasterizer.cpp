#include "LineRasterizer.h"

namespace sv {

ConservativeLineRasterizer::ConservativeLineRasterizer(const sf::Vec2i &begin, const sf::Vec2i &end)
{
	this->pos = begin;
	this->end = end;
	sf::Vec2i delta = end - begin;
	step.x = delta.x > 0 ? +1 : -1;
	step.y = delta.y > 0 ? +1 : -1;
	num = step * delta;
	t = sf::Vec2i(0);
}

sf::Vec2i ConservativeLineRasterizer::next()
{
	sf::Vec2i prevPos = pos;

	int64_t dx = (uint64_t)(t.x * 2 + 1) * num.y;
	int64_t dy = (uint64_t)(t.y * 2 + 1) * num.x;
	if (dx < dy) {
		pos.x += step.x;
		t.x++;
	} else if (dx > dy) {
		pos.y += step.y;
		t.y++;
	} else {
		pos += step;
		t += sf::Vec2i(1);
	}

	return prevPos;
}

}
