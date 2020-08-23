#pragma once

#include "client/gui/Gui.h"

namespace cl { namespace gui {

struct WidgetLinearLayout : WidgetBase<'l','l','a','y'>
{
	Direction direction = DirX;
	float marginBefore = 0.0f;
	float marginAfter = 0.0f;
	float padding = 0.0f;
	float anchor = 0.5f;

	virtual void layout(GuiLayout &layout, const sf::Vec2 &min, const sf::Vec2 &max) override;
};

} }
