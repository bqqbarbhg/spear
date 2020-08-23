#pragma once

#include "sf/Vector.h"
#include "client/InputState.h"

namespace cl {

struct GuiAction
{
	enum Flags
	{
		// Button
		MouseHover  = 0x01,
		MouseLeft   = 0x02,
		MouseRight  = 0x04,
		MouseMiddle = 0x08,
		Touch       = 0x10,

		// Action
		Tap         = 0x0100,
		DoubleTap   = 0x0200,
		Hold        = 0x0400,
		Drag        = 0x0800,
		Scroll      = 0x1000,

		// Meta
		ButtonMask  = 0xff,
		ACtionMask  = 0xff00,
	};

	uint32_t flags = 0;

	bool visible = true;

	float scrollDelta;

	sf::Vec2 pointer;
	sf::Vec2 pointerDelta;
};

struct GuiState
{
	void pushId(uint64_t id);
	void popId();

	void pushTransform(const sf::Vec2 &offset, const sf::Vec2 &scale=sf::Vec2(1.0f));
	void popTransform();

	void pushCrop(const sf::Vec2 &min, const sf::Vec2 &max);
	void popCrop();

	uint32_t pushArea(uint64_t id, const sf::Vec2 &pos, const sf::Vec2 &size);
	bool action(GuiAction &action, uint32_t guiAreaId, uint32_t flags);
};

}
