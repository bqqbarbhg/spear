#pragma once

#include "client/gui/Gui.h"

namespace cl { namespace gui {

struct WidgetGridLayout : WidgetBase<'g','l','a','y'>
{
	Direction direction = DirY;
	float margin = 0.0f;
	float padding = 0.0f;
	float anchor = 0.5f;

	virtual void layout(GuiLayout &layout, const sf::Vec2 &min, const sf::Vec2 &max) override;
};

} }
