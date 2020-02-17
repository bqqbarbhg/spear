#pragma once

#include "sf/Base.h"
#include "sf/Matrix.h"
#include "sf/Vector.h"
#include "sf/String.h"

namespace sp {

struct Sprite;
struct Font;

struct Canvas
{
	static Canvas *create();
	static void free(Canvas *c);

	void draw(uint32_t layer, Sprite *s, const sf::Mat23 &transform);
	void drawText(uint32_t layer, Font *f, sf::String str, const sf::Vec2 &pos);
};

}
