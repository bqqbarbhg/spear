#include "WidgetScroll.h"

namespace cl { namespace gui {

void WidgetScroll::layout(GuiLayout &layout, const sf::Vec2 &min, const sf::Vec2 &max)
{
	Direction du = direction, dv = dirOther(du);
	layoutSize = sf::clamp(boxExtent, min, max);

	float contentSizeU = 0.0f;
	for (Widget *w : children) {
		w->layout(layout, Zero2, dirVec2(du, Inf, layoutSize.v[dv]));
		contentSizeU += w->layoutSize.v[du];
	}

	float maxScroll = sf::max(contentSizeU - layoutSize.v[du], 0.0f);

	float velocity = 0.0f;
	if (dragged) {
		scrollOffset = scrollTarget;
		if (scrollOffset < 0.0f) {
			scrollOffset = -logf(1.0f - scrollOffset * (1.0f/overshootAmount)) * overshootAmount;
		} else if (scrollOffset > maxScroll) {
			scrollOffset = maxScroll + logf(1.0f + (scrollOffset - maxScroll) * (1.0f/overshootAmount)) * overshootAmount;
		}
		velocity = (scrollOffset - prevScrollOffset) / layout.dt;
	} else {
		scrollOffset += (smoothVelocity + dragVelocity) * layout.dt;
		float clamped = sf::clamp(scrollOffset, 0.0f, maxScroll);
		lerpExp(scrollOffset, clamped, 30.0f, 10.0f, layout.dt);
	}

	prevScrollOffset = scrollOffset;
	lerpExp(smoothVelocity, velocity, 20.0f, 10.0f, layout.dt);
	lerpExp(dragVelocity, 0.0f, 30.0f, 20.0f, layout.dt);

	float u = -scrollOffset;
	for (Widget *w : children) {
		w->layoutOffset = dirVec2(du, u, 0.0f);
		u += w->layoutSize.v[du];
	}

	prevDragged = dragged;
	dragged = false;
}

void WidgetScroll::paint(GuiPaint &paint)
{
	sf::Vec2 cropMin = layoutOffset, cropMax = layoutOffset + layoutSize;
	paint.canvas->pushCrop(cropMin, cropMax);

	CropRect oldCrop = paint.crop;
	paint.crop.min = sf::max(cropMin, paint.crop.min);
	paint.crop.max = sf::min(cropMax, paint.crop.max);

	Widget::paint(paint);

	paint.crop = oldCrop;
	paint.canvas->popCrop();
}

bool WidgetScroll::onPointer(GuiPointer &pointer)
{
	pointer.blocked = true;
	if (Widget::onPointer(pointer)) return true;
	Direction du = direction, dv = dirOther(du);

	if (pointer.button == GuiPointer::Touch && pointer.action == GuiPointer::Drag) {
		if (!prevDragged) scrollTarget = scrollOffset;
		scrollTarget -= pointer.delta.v[du] * pointer.dragFactor;
		pointer.trackWidget = sf::boxFromPointer(this);
		dragged = true;
		return true;
	}

	if (pointer.action == GuiPointer::Scroll) {
		dragVelocity += pointer.scrollDelta * scrollSpeed;
		return true;
	}

	return false;
}

} }
