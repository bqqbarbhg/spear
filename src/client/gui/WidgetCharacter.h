#pragma once

#include "client/gui/Gui.h"

#include "sp/Sprite.h"

namespace cl { namespace gui {

struct WidgetCharacter : WidgetBase<'c','h','a','r'>
{
	sp::SpriteRef icon;

	sf::SmallStringBuf<64> healthStr;

	int32_t currentHealth = 0;
	int32_t maxHealth = 0;
	float targetHealth = 0.0f;
	float smoothHealth = 0.0f;
	float targetTimer = 0.0f;

	int32_t prevCurrentHealth = -1;
	int32_t prevMaxHealth = -1;
	bool turnChanged = false;
	bool turnActive = false;
	bool clicked = false;

	virtual void layout(GuiLayout &layout, const sf::Vec2 &min, const sf::Vec2 &max) override;
	virtual void paint(GuiPaint &paint) override;
	virtual bool onPointer(GuiPointer &pointer) override;
};

} }
