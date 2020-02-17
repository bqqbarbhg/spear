#pragma once

#include "sf/Array.h"

namespace sp {

struct RectPacker
{
	struct Rect
	{
		uint16_t x, y, w, h;
		uint32_t childIndex;
	};

	sf::Array<Rect> rects;
	void reset(uint32_t width, uint32_t height);
	bool pack(uint32_t width, uint32_t height, uint32_t &x, uint32_t &y);
};

}
