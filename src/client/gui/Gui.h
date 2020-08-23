#pragma once

#include "sf/Vector.h"
#include "sp/Canvas.h"
#include "sf/Array.h"
#include "sf/Box.h"
#include "client/InputState.h"

namespace cl { namespace gui {

static const constexpr float Inf = 1e20f;

extern const sf::Vec2 Zero2;
extern const sf::Vec2 Inf2;

struct Widget;

enum Direction
{
	DirX,
	DirY,
};

sf_inline Direction dirOther(Direction d) { return (Direction)(d ^ 1); }

sf_inline sf::Vec2 dirVec2(Direction d, float u, float v=0.0f) {
	sf::Vec2 r;
	r.v[d] = u;
	r.v[d^1] = v;
	return r;
}

struct GuiPointer
{
	enum Button
	{
		MouseHover = Pointer::MouseHover,
		MouseLeft = Pointer::MouseLeft,
		MouseRight = Pointer::MouseRight,
		MouseMiddle = Pointer::MouseMiddle,
		Touch = Pointer::Touch,
	};

	enum Action
	{
		NoAction,
		Down,
		Up,
		Tap,
		DoubleTap,
		LongPress,
		Hold,
		Drag,
		Scroll,
	};

	Button button;
	Action action = NoAction;
	sf::Vec2 position;
	sf::Vec2 delta;
	float scrollDelta = 0.0f;
	float dragFactor = 0.0f;

	bool blocked = false;
	sf::Box<Widget> trackWidget;
};

struct GuiLayout
{
	float dt;
};

struct CropRect
{
	sf::Vec2 min;
	sf::Vec2 max;
};

struct GuiPaint
{
	sp::Canvas *canvas = nullptr;
	CropRect crop;
};

struct Widget
{
	sf::Array<sf::Box<Widget>> children;

	sf::Vec2 boxOffset;
	sf::Vec2 boxExtent = Inf2;

	sf::Vec2 layoutOffset;
	sf::Vec2 layoutSize;

	virtual void layout(GuiLayout &layout, const sf::Vec2 &min, const sf::Vec2 &max);
	virtual void paint(GuiPaint &paint);
	virtual bool onPointer(GuiPointer &pointer);

	static void finishLayout(sf::Array<Widget*> &workArray, Widget *root);
};

sf_inline void lerpExp(float &state, float target, float exponential, float linear, float dt)
{
	state = sf::lerp(target, state, exp2f(dt*-exponential));

	float speed = linear * dt;
	state += sf::clamp(target - state, -speed, speed);
}

} }
