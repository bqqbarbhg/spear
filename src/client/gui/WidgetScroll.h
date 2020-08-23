#pragma once

#include "client/gui/Gui.h"
#include "sp/Canvas.h"

namespace cl { namespace gui {

struct WidgetScroll : Widget
{
	Direction direction = DirY;

	float scrollOffset = 0.0f;
	float smoothVelocity = 0.0f;
	float dragVelocity = 0.0f;

	float scrollSpeed = 150.0f;

	float overshootAmount = 20.0f;
	float scrollTarget = 0.0f;
	float prevScrollOffset = 0.0f;
	bool prevDragged = false;
	bool dragged = false;

	virtual void layout(GuiLayout &layout, const sf::Vec2 &min, const sf::Vec2 &max) override;
	virtual void paint(GuiPaint &paint) override;

	virtual bool onPointer(GuiPointer &pointer) override;
};

} }
