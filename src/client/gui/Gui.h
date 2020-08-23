#pragma once

#include "sf/Vector.h"
#include "sp/Canvas.h"
#include "sf/Array.h"
#include "sf/Box.h"
#include "sf/Symbol.h"
#include "client/InputState.h"

namespace cl { namespace gui {

struct GuiResources;

static const constexpr float Inf = 1e20f;
static const constexpr uint64_t NoWidgetId = UINT64_C(0xf0f0f0f0f0f0f0f0);

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
		DropHover,
		DropCommit,
	};

	Button button;
	Action action = NoAction;
	sf::Vec2 position;
	sf::Vec2 delta;
	float scrollDelta = 0.0f;
	float dragFactor = 0.0f;

	bool blocked = false;
	sf::Box<Widget> trackWidget;
	sf::Symbol dropType;
	sf::Box<void> dropData;
};

struct GuiLayout
{
	float dt;
	GuiResources *resources = nullptr;
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
	GuiResources *resources = nullptr;
};

struct Widget
{
	sf::Array<sf::Box<Widget>> children;

	uint64_t id = NoWidgetId;
	uint32_t type;

	bool created = true;

	sf::Vec2 boxOffset;
	sf::Vec2 boxExtent = Inf2;

	sf::Vec2 layoutOffset;
	sf::Vec2 layoutSize;

	Widget(uint32_t type) : type(type) { }

	virtual void layout(GuiLayout &layout, const sf::Vec2 &min, const sf::Vec2 &max);
	virtual void paint(GuiPaint &paint);
	virtual bool onPointer(GuiPointer &pointer);

	static void finishLayout(sf::Array<Widget*> &workArray, Widget *root);

	sf_forceinline bool match(uint32_t type, uint64_t id) const {
		return this->type == type && this->id == id;
	}
};

template <char A, char B, char C, char D>
struct WidgetBase : Widget
{
	static constexpr const uint32_t WidgetType = (uint32_t)A | (uint32_t)B << 8 | (uint32_t)C << 16 | (uint32_t)D << 24;
	WidgetBase() : Widget(WidgetType) { }
};

sf_inline void lerpExp(float &state, float target, float exponential, float linear, float dt)
{
	state = sf::lerp(target, state, exp2f(dt*-exponential));

	float speed = linear * dt;
	state += sf::clamp(target - state, -speed, speed);
}

} }
