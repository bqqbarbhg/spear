#pragma once

#include "sf/Base.h"
#include "sf/Matrix.h"
#include "sf/Vector.h"
#include "sf/String.h"

namespace sp {

struct Sprite;
struct Font;

struct CanvasRenderOpts
{
	sf::Mat44 transform;
};

struct CanvasImp;
struct Canvas
{
	Canvas();
	~Canvas();
	Canvas(const Canvas &rhs) = delete;
	Canvas(Canvas &&rhs) : imp(rhs.imp) { rhs.imp = nullptr; }

	void clear();
	void draw(Sprite *s, const sf::Mat23 &transform);
	void drawText(Font *f, sf::String str, const sf::Vec2 &pos);
	void render(const CanvasRenderOpts &opts);

	CanvasImp *imp;
};

}
