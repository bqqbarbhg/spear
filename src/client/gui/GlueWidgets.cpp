#include "GlueWidgets.h"

namespace cl { namespace gui {

void WidgetSprite::paint(sp::Canvas &canvas, const sf::Vec2 &offset, const CropRect &crop)
{
	canvas.draw(sprite, offset, layoutSize);
}

} }
