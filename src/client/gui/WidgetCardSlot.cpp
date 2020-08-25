#include "WidgetCardSlot.h"
#include "client/gui/GuiResources.h"

namespace cl { namespace gui {

static const sf::Symbol guiCardSym { "GuiCard" };

void WidgetCardSlot::layout(GuiLayout &layout, const sf::Vec2 &min, const sf::Vec2 &max)
{
	sf::Vec2 size = sf::clamp(boxExtent, min, max);
	float scale = sf::min(size.x * GuiCard::canvasYByX, size.y);
	layoutSize = sf::Vec2(scale * GuiCard::canvasXByY, scale);

	if (cardSwitchTime > 0.0f) {
		cardSwitchTime -= layout.dt * 5.0f;
		if (cardSwitchTime <= 0.0f) {
			prevCard.reset();
			cardSwitchTime = 0.0f;
		}
	} else {
		if (card != prevCard) {
			if (prevCard) {
				dropOutline = 0.0f;
				cardSwitchTime = 1.0f;
				if (card && card->prevSlotIndex != ~0u && card->prevSlotIndex != slotIndex && layout.frameIndex - card->prevSlotFrame < 5) {
					cardSwitchDirection.x = slotIndex < card->prevSlotIndex ? +1.0f : -1.0f;
					cardSwitchDirection.y = slotIndex < card->prevSlotIndex ? +0.2f : -0.3f;
				} else {
					cardSwitchDirection.x = 0.0f;
					cardSwitchDirection.y = -1.0f;
				}
			}
		}
		prevCard = card;
	}

	if (card) {
		card->prevSlotIndex = slotIndex;
		card->prevSlotFrame = layout.frameIndex;
	}

	if (startAnim > 0.0f) {
		startAnim -= layout.dt * 4.0f;
		if (startAnim <= 0.0f) {
			startAnim = 0.0f;
		}
	}

	prevDragTimer = dragTimer;
	if (dragTimer >= 0.0f) {
		if (card != draggedCard) {
			dragTimer = 0.0f;
		}
		dragTimer -= layout.dt * 3.0f;
	}

	if (selected) {
		selectTime = sf::min(selectTime + layout.dt * 7.0f, 1.0f);
	} else if (selectTime > 0.0f) {
		selectTime = sf::max(selectTime - layout.dt * 7.0f, 0.0f);
	}

	if (dropHover) {
		dropOutline = sf::min(dropOutline + layout.dt * 7.0f, 1.0f);
	} else if (dropOutline > 0.0f) {
		dropOutline = sf::max(dropOutline - layout.dt * 5.0f, 0.0f);
	}
	dropHover = false;

	if (pressTimer > 0.0f) {
		pressAnim = sf::min(pressAnim + layout.dt * 7.0f, 1.0f);
		pressTimer -= layout.dt;
	} else if (pressAnim > 0.0f) {
		pressAnim = sf::max(pressAnim - layout.dt * 5.0f, 0.0f);
	}

	wantSelect = false;
}

void WidgetCardSlot::paint(GuiPaint &paint)
{
	if (startAnim > 0.0f) {
		float t = smoothBegin(sf::min(1.0f, startAnim));
		paint.canvas->pushTransform(sf::mat2D::translateY(layoutSize.y*0.25f*t));
		paint.canvas->pushTint(sf::Vec4(1.0f - t));
	}

	if (selectTime > 0.0f || pressAnim > 0.0f) {
		float t = smoothStep(sf::min(1.0f, selectTime));
		float u = smoothStep(sf::min(1.0f, pressAnim));
		float offset = -0.15f * t + 0.05f * u;
		paint.canvas->pushTransform(sf::mat2D::translateY(layoutSize.y*offset));
	}

	if (card) {
		if (cardSwitchTime > 0.0f && prevCard) {
			sf::Mat23 t;
			t.m00 = layoutSize.x * (1.0f/500.0f);
			t.m11 = layoutSize.y * (1.0f/800.0f);
			t.m02 = layoutOffset.x;
			t.m12 = layoutOffset.y;
			paint.canvas->pushTransform(t);
			renderCard(*paint.canvas, *prevCard);

			sf::Vec4 col = sf::Vec4(0.0f, 0.0f, 0.0f, 0.6f);
			paint.canvas->draw(paint.resources->cardSilhouette, sf::Vec2(), sf::Vec2(500.0f, 800.0f), col);

			paint.canvas->popTransform();
		}

		sf::Vec2 dropOffset;
		if (cardSwitchTime > 0.0f) {
			float t = smoothBegin(cardSwitchTime) * layoutSize.y * 0.05f;
			dropOffset = cardSwitchDirection * t;
		}

		sf::Mat23 t;
		t.m00 = layoutSize.x * (1.0f/500.0f);
		t.m11 = layoutSize.y * (1.0f/800.0f);
		t.m02 = layoutOffset.x + dropOffset.x;
		t.m12 = layoutOffset.y + dropOffset.y;
		paint.canvas->pushTransform(t);
		renderCard(*paint.canvas, *card);

		if (prevDragTimer > 0.0f) {
			sf::Vec4 col = sf::Vec4(0.0f, 0.0f, 0.0f, smoothStep(prevDragTimer) * 0.6f);
			paint.canvas->draw(paint.resources->cardSilhouette, sf::Vec2(), sf::Vec2(500.0f, 800.0f), col);
		}

		paint.canvas->popTransform();
	} else if ((uint32_t)slot < (uint32_t)GuiCardSlot::Count) {
		sp::Sprite *sprite = paint.resources->slotPlaceholders[(uint32_t)slot];
		paint.canvas->draw(sprite, layoutOffset, layoutSize);
	}

	if (dropOutline > 0.0f) {
		float a = smoothStep(dropOutline) * 0.8f;
		sf::Vec4 col = sf::Vec4(0.6f, 0.6f, 1.0f, 1.0f) * a;
		paint.canvas->draw(paint.resources->cardOutline, layoutOffset, layoutSize, col);
	}

	if (selectTime > 0.0f || pressAnim > 0.0f) {
		paint.canvas->popTransform();
	}

	if (startAnim > 0.0f) {
		paint.canvas->popTint();
		paint.canvas->popTransform();
	}

}

bool WidgetCardSlot::onPointer(GuiPointer &pointer)
{
	pointer.blocked = true;
	if (pointer.trackWidget == this) {
		if (draggedCard && draggedCard == card) {
			pointer.dropType = guiCardSym;
			pointer.dropData = draggedCard;
			pointer.dropOffset = layoutOffset;
			pointer.dropSize = layoutSize;
			dragTimer = 1.0f;
		}
		return true;
	}

	if (card) {
		if ((pointer.button == GuiPointer::MouseLeft || pointer.button == GuiPointer::Touch) && pointer.action == GuiPointer::Tap) {
			wantSelect = true;
			return true;
		} else if (pointer.button == GuiPointer::MouseLeft && (pointer.action == GuiPointer::Drag || pointer.action == GuiPointer::LongPress)) {
			pointer.trackWidget = sf::boxFromPointer(this);
			draggedCard = card;
			dragTimer = 1.0f;
			pointer.dropType = guiCardSym;
			pointer.dropData = card;
			pointer.dropOffset = layoutOffset;
			pointer.dropSize = layoutSize;
			return true;
		} else if (pointer.button == GuiPointer::Touch && pointer.action == GuiPointer::LongPress) {
			pointer.trackWidget = sf::boxFromPointer(this);
			draggedCard = card;
			dragTimer = 1.0f;
			pointer.dropType = guiCardSym;
			pointer.dropData = card;
			pointer.dropOffset = layoutOffset;
			pointer.dropSize = layoutSize;
			return true;
		} else if ((pointer.button == GuiPointer::MouseLeft || pointer.button == GuiPointer::Touch) && (pointer.action == GuiPointer::Down || pointer.action == GuiPointer::Hold)) {
			if (pressTimer < 0.0f) {
				pressTimer = 0.1f;
			} else {
				pressTimer = sf::max(pressTimer, 0.01f);
			}
		}
	}

	if (pointer.action == GuiPointer::DropHover && pointer.dropType == guiCardSym) {
		GuiCard *newCard = pointer.dropData.cast<GuiCard>();
		if (newCard->slot == slot) {
			dropHover = true;
		}
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
