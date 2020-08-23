#include "WidgetGridLayout.h"

namespace cl { namespace gui {

void WidgetGridLayout::layout(GuiLayout &layout, const sf::Vec2 &min, const sf::Vec2 &max)
{
	Direction du = direction, dv = dirOther(du);
	float extentV = sf::max(min.v[dv], sf::min(boxExtent.v[dv], max.v[dv] - 2.0f*margin));

	float u = margin;
	float v = 0.0f;
	float colU = 0.0f;
	for (Widget *w : children) {
		w->layout(layout, Zero2, dirVec2(du, Inf, extentV));
		colU = sf::max(colU, w->layoutSize.v[du]);

		float newV = v + w->layoutSize.v[dv];
		if (newV > extentV + 0.1f) {
			u += colU + padding;
			colU = 0.0f;
			w->layoutOffset.v[du] = u;
			w->layoutOffset.v[dv] = margin;
			v = w->layoutSize.v[dv] + padding;
		} else {
			w->layoutOffset.v[du] = u;
			w->layoutOffset.v[dv] = v + margin;
			v = newV + padding;
		}

	}

	layoutSize = dirVec2(du, u + colU, extentV);
}

} }
