#pragma once

#include "client/gui/Gui.h"
#include "sp/Sprite.h"

namespace cl { namespace gui {

struct WidgetSprite : WidgetBase<'g','s','p','r'>
{
	sp::SpriteRef sprite;

	virtual void paint(GuiPaint &paint) override;
};

struct WidgetToggleButton : WidgetBase<'g','t','g','l'>
{
	sp::SpriteRef inactiveSprite;
	sp::SpriteRef activeSprite;

	bool active = false;

	virtual void paint(GuiPaint &paint) override;
	virtual bool onPointer(GuiPointer &pointer) override;
};


} }
