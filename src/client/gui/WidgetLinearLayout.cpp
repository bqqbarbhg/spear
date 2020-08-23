#include "WidgetLinearLayout.h"

namespace cl { namespace gui {

void WidgetLinearLayout::layout(GuiLayout &layout, const sf::Vec2 &min, const sf::Vec2 &max)
{
	Direction du = direction, dv = dirOther(du);
	float extentV = sf::clamp(boxExtent.v[dv], min.v[dv], max.v[dv]);

	float u = marginBefore;
	for (Widget *w : children) {
		w->layout(layout, Zero2, dirVec2(du, Inf, extentV));
		w->layoutOffset = dirVec2(du, u, (extentV - w->layoutSize.v[dv]) * anchor);
		u += w->layoutSize.v[du] + padding;
	}
	u += marginAfter;

	layoutSize = dirVec2(du, u, extentV);
}


} }
