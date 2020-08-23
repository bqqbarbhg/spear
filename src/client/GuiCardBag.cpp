#if 0
#include "GuiCardBag.h"

#include "client/GuiState.h"

#include "ext/sokol/sokol_gfx.h"


namespace cl {

void GuiCardBag::update(GuiState &guiState, const FrameArgs &frameArgs, const GuiArgs &guiArgs)
{
	guiState.pushTransform(offset);
	guiState.pushCrop(sf::Vec2(), size);

	uint32_t bgArea = guiState.pushArea(1, sf::Vec2(), size);
	GuiAction action;
	if (guiState.action(action, bgArea, GuiAction::Touch|GuiAction::Drag)) {
		scrollY += action.pointerDelta.y;
	}
	if (guiState.action(action, bgArea, GuiAction::Scroll)) {
		scrollY += action.scrollDelta;
	}

	guiState.pushTransform(sf::Vec2(0.0f, -scrollY));



	guiState.popTransform();

	guiState.popCrop();
	guiState.popTransform();
}

void GuiCardBag::render(const FrameArgs &frameArgs, const GuiArgs &guiArgs)
{
	sf::Vec2 guiToPixel = sf::Vec2(frameArgs.resolution) / guiArgs.resolution;
	sf::Vec2i pixelOffset = sf::Vec2i(offset * guiToPixel);
	sf::Vec2i pixelSize = sf::Vec2i(size * guiToPixel);

	sg_apply_scissor_rect(pixelOffset.x, pixelOffset.y, pixelSize.x, pixelSize.y, true);



	sg_apply_scissor_rect(0, 0, frameArgs.resolution.x, frameArgs.resolution.y, true);
}
}

#endif