#include "GlueWidgets.h"

namespace cl { namespace gui {

void WidgetSprite::paint(GuiPaint &paint)
{
	paint.canvas->draw(sprite, layoutOffset, layoutSize);
}

void WidgetButton::layout(GuiLayout &layout, const sf::Vec2 &min, const sf::Vec2 &max)
{
	Widget::layout(layout, min, max);
	pressed = false;
}

void WidgetButton::paint(GuiPaint &paint)
{
	sf::Vec4 *pColor = &spriteColor;
	if (impHovered) pColor = &spriteColorHover;
	if (impPressed) pColor = &spriteColorPress;
	impHovered = false;
	impPressed = false;

	paint.canvas->draw(sprite, layoutOffset, layoutSize, *pColor);

	if (font && text) {
		sf::Vec2 textSize = font->measureText(text, fontHeight);

		sp::TextDraw td;
		td.font = font;
		td.string = text;
		td.height = fontHeight;
		td.transform.m02 = layoutOffset.x + (layoutSize.x - textSize.x) * 0.5f;
		td.transform.m12 = layoutOffset.y + layoutSize.y * 0.5f + fontHeight * 0.25f;
		td.color = fontColor;
		paint.canvas->drawText(td);
	}
}

bool WidgetButton::onPointer(GuiPointer &pointer)
{
	pointer.blocked = true;
	if (pointer.button == GuiPointer::MouseLeft || pointer.button == GuiPointer::Touch) {
		if (pointer.action == GuiPointer::Down) {
			pressed = true;
			impPressed = true;
			return true;
		} else if (pointer.action == GuiPointer::Hold || pointer.action == GuiPointer::LongPress) {
			impPressed = true;
			return true;
		}
	} else if (pointer.button == GuiPointer::MouseHover) {
		impHovered = true;
		return true;
	}
	return false;
}

void WidgetToggleButton::paint(GuiPaint &paint)
{
	sp::Sprite *sprite = active ? (sp::Sprite*)activeSprite : (sp::Sprite*)inactiveSprite;
	paint.canvas->draw(sprite, layoutOffset, layoutSize);
}

bool WidgetToggleButton::onPointer(GuiPointer &pointer)
{
	pointer.blocked = true;
	if (pointer.action == GuiPointer::Down) {
		if (pointer.button == GuiPointer::MouseLeft || pointer.button == GuiPointer::Touch) {
			active = !active;
			return true;
		}
	}
	return false;
}

void WidgetRichText::layout(GuiLayout &layout, const sf::Vec2 &min, const sf::Vec2 &max)
{
	sf::Vec2 size = sf::clamp(boxExtent, min, max);
	impWrapWidth = size.x;

	sp::RichTextDesc desc = { };
	desc.style = style;
	desc.wrapWidth = size.x;
	desc.baseColor = baseColor;
	desc.fontHeight = fontHeight;

	sp::RichTextMeasure measure = sp::measureRichText(desc, text);
	float width = measure.numLines > 1 ? desc.wrapWidth : measure.maxLineWidth;
	float height = measure.height;

	layoutSize = sf::clamp(sf::Vec2(width, height), min, max);
	for (Widget *w : children) {
		w->layout(layout, Zero2, layoutSize);
		w->layoutOffset = w->boxOffset;
	}
}

void WidgetRichText::paint(GuiPaint &paint)
{
	sp::RichTextDesc desc = { };
	desc.style = style;
	desc.wrapWidth = impWrapWidth;
	desc.baseColor = baseColor;
	desc.fontHeight = fontHeight;
	desc.offset = layoutOffset + sf::Vec2(0.0f, fontHeight * 0.7f);

	sp::drawRichText(*paint.canvas, desc, text);
}

void WidgetBlockPointer::layout(GuiLayout &layout, const sf::Vec2 &min, const sf::Vec2 &max)
{
	if (children.size == 1) {
		Widget *w = children[0];
		w->layout(layout, min, max);
		w->layoutOffset = w->boxOffset;
		layoutSize = w->boxOffset + w->layoutSize;
	} else {
		Widget::layout(layout, min, max);
	}
}

bool WidgetBlockPointer::onPointer(GuiPointer &pointer)
{
	pointer.blocked = true;
	return Widget::onPointer(pointer);
}

} }
