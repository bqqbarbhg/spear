#include "Gui.h"

namespace cl { namespace gui {

const sf::Vec2 Zero2 = { };
const sf::Vec2 Inf2 = { Inf, Inf };

void Widget::layout(GuiLayout &layout, const sf::Vec2 &min, const sf::Vec2 &max)
{
	layoutSize = sf::clamp(boxExtent, min, max);

	for (Widget *w : children) {
		w->layout(layout, Zero2, layoutSize);
		w->layoutOffset = w->boxOffset;
	}
}

void Widget::paint(GuiPaint &paint)
{
	for (Widget *w : children) {
		sf::Vec2 wMin = w->layoutOffset;
		sf::Vec2 wMax = wMin + w->layoutSize;
		if (wMax.x < paint.crop.min.x || wMax.y < paint.crop.min.y || wMin.x > paint.crop.max.x || wMin.y > paint.crop.max.y) continue;
		w->paint(paint);
	}
}

bool Widget::onPointer(GuiPointer &pointer)
{
	for (Widget *w : children) {
		sf::Vec2 wMin = w->layoutOffset;
		sf::Vec2 wMax = wMin + w->layoutSize;
		if (pointer.position.x < wMin.x || pointer.position.y < wMin.y || pointer.position.x >= wMax.x || pointer.position.y >= wMax.y) continue;

		if (w->onPointer(pointer)) {
			return true;
		}
	}

	return false;
}

void Widget::finishLayout(sf::Array<Widget*> &workArray, Widget *root)
{
	workArray.clear();
	workArray.push(root);
	while (workArray.size > 0) {
		Widget *parent = workArray.popValue();

		sf::Vec2 offset = parent->layoutOffset;
		for (Widget *w : parent->children) {
			w->layoutOffset += offset;
			if (w->children.size > 0) {
				workArray.push(w);
			}
		}
	}
}

} }
