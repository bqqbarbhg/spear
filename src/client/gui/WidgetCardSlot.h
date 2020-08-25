#pragma once

#include "client/gui/Gui.h"
#include "client/GuiCard.h"

namespace cl { namespace gui {

struct WidgetCardSlot : WidgetBase<'s','l','o','t'>
{
	GuiCardSlot slot = GuiCardSlot::Count;
	sf::Box<GuiCard> card;
	sf::Box<GuiCard> draggedCard;
	sf::Box<GuiCard> droppedCard;
	sf::Box<GuiCard> prevCard;
	uint32_t slotIndex = ~0u;
	float dragTimer = 0.0f;
	float prevDragTimer = 0.0f;
	float dropOutline = 0.0f;
	float cardSwitchTime = 0.0f;
	sf::Vec2 cardSwitchDirection;
	bool dropHover = false;
	bool wantSelect = false;
	bool selected = false;
	float pressTimer = 0.0f;
	float selectTime = 0.0f;
	float startAnim = 0.0f;
	float pressAnim = 0.0f;

	virtual void layout(GuiLayout &layout, const sf::Vec2 &min, const sf::Vec2 &max) override;
	virtual void paint(GuiPaint &paint) override;
	virtual bool onPointer(GuiPointer &pointer) override;
};

} }
