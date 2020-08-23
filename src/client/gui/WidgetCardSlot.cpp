#include "WidgetCardSlot.h"
#include "client/gui/GuiResources.h"

namespace cl { namespace gui {

const sf::Symbol guiCardSym { "GuiCard" };

void WidgetCardSlot::layout(GuiLayout &layout, const sf::Vec2 &min, const sf::Vec2 &max)
{
	sf::Vec2 size = sf::clamp(boxExtent, min, max);
	float scale = sf::min(size.x * GuiCard::canvasYByX, size.y);
	layoutSize = sf::Vec2(scale * GuiCard::canvasXByY, scale);
}

void WidgetCardSlot::paint(GuiPaint &paint)
{
	if (card && !dragged) {
		sf::Mat23 t;
		t.m00 = layoutSize.x * (1.0f/500.0f);
		t.m11 = layoutSize.y * (1.0f/800.0f);
		t.m02 = layoutOffset.x;
		t.m12 = layoutOffset.y;
		paint.canvas->pushTransform(t);
		renderCard(*paint.canvas, *card);
		paint.canvas->popTransform();
	} else if ((uint32_t)slot < (uint32_t)GuiCardSlot::Count) {
		sp::Sprite *sprite = paint.resources->slotPlaceholders[(uint32_t)slot];
		paint.canvas->draw(sprite, layoutOffset, layoutSize);
	}
}

bool WidgetCardSlot::onPointer(GuiPointer &pointer)
{
	if (pointer.trackWidget == this) {
		if (card) {
			pointer.dropType = guiCardSym;
			pointer.dropData = card;
		}
		return true;
	}

	if (card) {
		if (pointer.button == GuiPointer::MouseLeft && pointer.action == GuiPointer::Drag) {
			pointer.trackWidget = sf::boxFromPointer(this);
			pointer.dropType = guiCardSym;
			pointer.dropData = card;
			return true;
		} else if (pointer.button == GuiPointer::Touch && pointer.action == GuiPointer::LongPress) {
			pointer.trackWidget = sf::boxFromPointer(this);
			pointer.dropType = guiCardSym;
			pointer.dropData = card;
			return true;
		}
	}

	if (pointer.action == GuiPointer::DropHover && pointer.dropType == guiCardSym) {
		return true;
	} else if (pointer.action == GuiPointer::DropCommit && pointer.dropType == guiCardSym) {
		sf::Box<GuiCard> newCard = pointer.dropData.cast<GuiCard>();
		if (newCard->slot == slot && newCard != card) {
			droppedCard = newCard;
		}
		return true;
	}

	return false;
}

} }
