#pragma once

#include "client/gui/Gui.h"
#include "sp/Sprite.h"

namespace cl { namespace gui {

struct WidgetSprite : Widget
{
	sp::SpriteRef sprite;

	virtual void paint(sp::Canvas &canvas, const sf::Vec2 &offset, const CropRect &crop);
};


} }
