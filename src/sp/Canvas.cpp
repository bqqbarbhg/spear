#include "Canvas.h"

namespace sp {

struct CanvasImp : Canvas
{
};

Canvas *Canvas::create()
{
	CanvasImp *imp = new CanvasImp();
	return imp;
}

void Canvas::free(Canvas *c)
{
	CanvasImp *imp = (CanvasImp*)c;
	delete imp;
}

void Canvas::draw(uint32_t layer, Sprite *s, const sf::Mat23 &transform)
{
	CanvasImp *imp = (CanvasImp*)this;
}

void Canvas::drawText(uint32_t layer, Font *f, sf::String str, const sf::Vec2 &pos)
{
	CanvasImp *imp = (CanvasImp*)this;
}

}
