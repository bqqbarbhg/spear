#include "GlueWidgets.h"

namespace cl { namespace gui {

void WidgetSprite::paint(GuiPaint &paint)
{
	paint.canvas->draw(sprite, layoutOffset, layoutSize);
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

} }
