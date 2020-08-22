#pragma once

#include "sf/Vector.h"
#include "sf/Matrix.h"
#include "sf/Array.h"
#include "sf/Geometry.h"
#include "sf/String.h"

#include "ext/sokol/sokol_app.h"

struct sapp_event;

namespace cl {

struct PointerPosition
{
	sf::Vec2 pos; // Relative position in screen, normalized to [(0,0),(1,1)]
	sf::Ray worldRay; // World space camera ray
};

struct Pointer
{
	enum Button
	{
		MouseHover,
		MouseLeft,
		MouseRight,
		MouseMiddle,
		Touch,

		LastMouse = MouseMiddle,
	};

	enum Action
	{
		Down,
		Hold,
		Up,
		Cancel,
	};

	uint64_t id = 0;
	uintptr_t touchId = 0;
	Button button;
	Action action;

	PointerPosition start; // Initial state when clicked/tapped
	PointerPosition dragStart; // Resampled when touches are lifted
	PointerPosition current; // Current frame state

	float time = 0.0f;
	bool canTap = true;
	float dragAmount = -0.1f;

	void formatDebugString(sf::StringBuf &str) const;
};

struct InputUpdateArgs
{
	float dt = 0.0f;
	sf::Vec2 resolution;
	sf::Mat44 clipToWorld;
	float dpiScale = 1.0f;
	sf::Slice<const sapp_event> events;
	bool mouseBlocked = false;
	bool keyboardBlocked = false;
	bool simulateTap = false;
};

struct InputState
{
	uint64_t nextPointerId = 0;
	uintptr_t simulatedTouchId = 0;
	uintptr_t nextSimulatedTouchId = 0;


	sf::Mat44 clipToWorld;
	sf::Vec2 rcpResolution = sf::Vec2(1.0f);
	sf::Vec2 dpiResolution = sf::Vec2(1.0f);

	// API
	bool mouseBlocked = false;
	bool keyboardBlocked = false;

	sf::Array<Pointer> pointers;
	bool keyDown[SAPP_MAX_KEYCODES] = { };
	bool prevKeyDown[SAPP_MAX_KEYCODES] = { };

	void update(const InputUpdateArgs &args);

	Pointer *find(uint64_t id);
};

sf::Ray pointerToWorld(const sf::Mat44 &clipToWorld, const sf::Vec2 &relativePos);

}
