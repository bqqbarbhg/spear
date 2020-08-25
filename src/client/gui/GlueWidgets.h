#pragma once

#include "client/gui/Gui.h"
#include "sp/Sprite.h"
#include "sp/Font.h"

namespace cl { namespace gui {

struct WidgetSprite : WidgetBase<'g','s','p','r'>
{
	sp::SpriteRef sprite;

	virtual void paint(GuiPaint &paint) override;
};

struct WidgetButton : WidgetBase<'g','b','t','n'>
{
	sp::SpriteRef sprite;
	sf::Vec4 spriteColor = sf::Vec4(1.0f);

	sp::FontRef font;
	float fontHeight;
	sf::Vec4 fontColor = sf::Vec4(0.0f, 0.0f, 0.0f, 1.0f);
	sf::Symbol text;

	bool pressed = false;

	virtual void layout(GuiLayout &layout, const sf::Vec2 &min, const sf::Vec2 &max) override;
	virtual void paint(GuiPaint &paint) override;
	virtual bool onPointer(GuiPointer &pointer) override;
};

struct WidgetToggleButton : WidgetBase<'g','t','g','l'>
{
	sp::SpriteRef inactiveSprite;
	sp::SpriteRef activeSprite;

	bool active = false;

	virtual void paint(GuiPaint &paint) override;
	virtual bool onPointer(GuiPointer &pointer) override;
};

struct WidgetBlockPointer : WidgetBase<'g','b','l','k'>
{
	virtual void layout(GuiLayout &layout, const sf::Vec2 &min, const sf::Vec2 &max) override;
	virtual bool onPointer(GuiPointer &pointer) override;
};


} }
