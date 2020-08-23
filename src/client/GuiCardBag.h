#pragma once

#include "sp/Canvas.h"
#include "client/System.h"

namespace cl {

struct GuiState;

struct GuiCardBag
{
	sp::Canvas canvas;
	uint32_t numColumns = 4;

	float scrollY = 0.0f;

	sf::Vec2 offset;
	sf::Vec2 size;

	void update(GuiState &guiState, const FrameArgs &frameArgs, const GuiArgs &guiArgs);

	void render(const FrameArgs &frameArgs, const GuiArgs &guiArgs);
};

}
