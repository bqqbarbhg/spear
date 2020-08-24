#include "WidgetCard.h"

#include "client/gui/GuiResources.h"

namespace cl { namespace gui {

static const sf::Symbol guiCardSym { "GuiCard" };

void WidgetCard::layout(GuiLayout &layout, const sf::Vec2 &min, const sf::Vec2 &max)
{
	sf::Vec2 size = sf::clamp(boxExtent, min, max);
	float scale = sf::min(size.x * GuiCard::canvasYByX, size.y);
	layoutSize = sf::Vec2(scale * GuiCard::canvasXByY, scale);

	prevDragTimer = dragTimer;
	if (dragTimer >= 0.0f) {
		if (card != draggedCard) {
			dragTimer = 0.0f;
		}
		dragTimer -= layout.dt * 3.0f;
	}
}

void WidgetCard::paint(GuiPaint &paint)
{
	if (card) {
		sf::Mat23 t;
		t.m00 = layoutSize.x * (1.0f/500.0f);
		t.m11 = layoutSize.y * (1.0f/800.0f);
		t.m02 = layoutOffset.x;
		t.m12 = layoutOffset.y;
		paint.canvas->pushTransform(t);
		renderCard(*paint.canvas, *card);

		if (prevDragTimer > 0.0f) {
			sf::Vec4 col = sf::Vec4(0.0f, 0.0f, 0.0f, smoothStep(prevDragTimer) * 0.6f);
			paint.canvas->draw(paint.resources->cardSilhouette, sf::Vec2(), sf::Vec2(500.0f, 800.0f), col);
		}

		paint.canvas->popTransform();
	} else {
		sp::Sprite *sprite = paint.resources->inventorySlot;
		paint.canvas->draw(sprite, layoutOffset, layoutSize);
	}
}

bool WidgetCard::onPointer(GuiPointer &pointer)
{
	if (pointer.trackWidget == this) {
		if (draggedCard && draggedCard == card) {
			pointer.dropType = guiCardSym;
			pointer.dropData = draggedCard;
			pointer.dropOffset = layoutOffset;
			pointer.dropSize = layoutSize;
		}
		dragTimer = 1.0f;
		return true;
	}

	if (card) {
		if (pointer.button == GuiPointer::MouseLeft && (pointer.action == GuiPointer::Drag || pointer.action == GuiPointer::LongPress)) {
			pointer.trackWidget = sf::boxFromPointer(this);
			pointer.dropType = guiCardSym;
			pointer.dropData = card;
			pointer.dropOffset = layoutOffset;
			pointer.dropSize = layoutSize;
			draggedCard = card;
			dragTimer = 1.0f;
			return true;
		} else if (pointer.button == GuiPointer::Touch && pointer.action == GuiPointer::LongPress) {
			pointer.trackWidget = sf::boxFromPointer(this);
			pointer.dropType = guiCardSym;
			pointer.dropData = card;
			pointer.dropOffset = layoutOffset;
			pointer.dropSize = layoutSize;
			draggedCard = card;
			dragTimer = 1.0f;
			return true;
		}
	}

	return false;
}

} }
