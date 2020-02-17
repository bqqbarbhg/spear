#include "RectPacker.h"

namespace sp {

void RectPacker::reset(uint32_t width, uint32_t height)
{
	rects.clear();
	Rect &root = rects.push();
	root.x = 0;
	root.y = 0;
	root.w = width;
	root.h = height;
	root.childIndex = 0;
}

bool RectPacker::pack(uint32_t w, uint32_t h, uint32_t &x, uint32_t &y)
{
	sf_assert(rects.size > 0);

	Rect *rs = rects.data;
	uint32_t stack[32], *stackEnd = stack + sf_arraysize(stack);
	uint32_t *top = stack;

	// Fail immediately if the root is bad
	if (w > rs[0].w || h > rs[0].h || rs[0].childIndex == ~0u) {
		return false;
	}

	uint32_t ix = 0;
	for (;;) {
		Rect &rect = rs[ix];
		uint32_t childIndex = rect.childIndex;
		if (childIndex > 0) {

			// Split node: Recurse into one or both 
			Rect *child = rects.data + childIndex;
			bool ok0 = child[0].w >= w && child[0].h >= h && child[0].childIndex != ~0u;
			bool ok1 = child[1].w >= w && child[1].h >= h && child[1].childIndex != ~0u;
			if (ok0) {
				if (ok1) {
					// Both children are suitable, need to push the other one
					// for later
					*top++ = childIndex + 1;
				}

				// Recurse left
				ix = childIndex + 0;
				continue;
			} else if (ok1) {
				// Recurse right
				ix = childIndex + 1;
				continue;
			}

		} else {
			uint32_t dw = rect.w - w;
			uint32_t dh = rect.h - h;
			if (dw + dh == 0) {
				x = rect.x;
				y = rect.y;
				rect.childIndex = ~0u;
				return true;
			}

			// Prepare to recurse into children
			ix = rects.size;

			// Setup child rectangles depending on the aspect ratio
			Rect *child = rects.pushUninit(2);
			rs = rects.data;

			child[0].x = rect.x;
			child[0].y = rect.y;
			child[0].childIndex = 0;
			child[1].childIndex = 0;
			if (dw > dh) {
				// Horizontal split
				child[0].w = w;
				child[0].h = rect.h;
				child[1].x = rect.x + w;
				child[1].y = rect.y;
				child[1].w = rect.w - w;
				child[1].h = rect.h;
			} else {
				// Vertical split
				child[0].w = rect.w;
				child[0].h = h;
				child[1].x = rect.x;
				child[1].y = rect.y + h;
				child[1].w = rect.w;
				child[1].h = rect.h - h;
			}
			continue;
		}

		if (top > stack) {
			ix = *--top;
		} else {
			return false;
		}
	}
}

}
