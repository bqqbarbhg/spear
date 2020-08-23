#include "InputState.h"

#include "ext/sokol/sokol_app.h"

namespace cl {

void Pointer::formatDebugString(sf::StringBuf &str) const
{
	const char *buttonStr, *actionStr;
	switch (button) {
	case MouseHover: buttonStr = "Hover"; break;
	case MouseLeft: buttonStr = "Left"; break;
	case MouseRight: buttonStr = "Right"; break;
	case MouseMiddle: buttonStr = "Middle"; break;
	case Touch: buttonStr = "Touch"; break;
	default: buttonStr = "(???)"; break;
	}

	switch (action) {
	case Down: actionStr = "Down"; break;
	case Hold: actionStr = "Hold"; break;
	case Up: actionStr = "Up"; break;
	case Cancel: actionStr = "Cancel"; break;
	default: actionStr = "(???)"; break;
	}

	str.format("(%2.2fs) %s %s - (%.2f, %.2f) [drag:%+.2f]", time, buttonStr, actionStr,
		current.pos.x, current.pos.y, dragFactor);
}

static Pointer::Button sappToPointerButton(sapp_mousebutton button)
{
	sf_assert(button >= SAPP_MOUSEBUTTON_LEFT && button <= SAPP_MOUSEBUTTON_MIDDLE);
	return (Pointer::Button)((uint32_t)button + 1);
}

sf::Ray pointerToWorld(const sf::Mat44 &clipToWorld, const sf::Vec2 &relativePos)
{
	sf::Ray ray;
	sf::Vec2 clipMouse = relativePos * sf::Vec2(+2.0f, -2.0f) + sf::Vec2(-1.0f, +1.0f);
	sf::Vec4 rayBegin = clipToWorld * sf::Vec4(clipMouse.x, clipMouse.y, 0.0f, 1.0f);
	sf::Vec4 rayEnd = clipToWorld * sf::Vec4(clipMouse.x, clipMouse.y, 1.0f, 1.0f);
	ray.origin = sf::Vec3(rayBegin.v) / rayBegin.w;
	ray.direction = sf::normalize(sf::Vec3(rayEnd.v) / rayEnd.w - ray.origin);
	return ray;
}

static PointerPosition sappToPointerPosition(InputState &state, const sf::Vec2 &pos)
{
	PointerPosition p;
	p.pos = state.rcpResolution * pos;
	p.worldRay = pointerToWorld(state.clipToWorld, p.pos);
	return p;
}

static Pointer &findOrAddButton(InputState &state, Pointer::Button button)
{
	for (Pointer &p : state.pointers) {
		if (p.button == button) {
			return p;
		}
	}

	Pointer &p = state.pointers.push();
	p.button = button;
	return p;
}

static Pointer *findButton(InputState &state, Pointer::Button button)
{
	for (Pointer &p : state.pointers) {
		if (p.button == button) {
			return &p;
		}
	}
	return nullptr;
}

static Pointer &findOrAddTouch(InputState &state, uintptr_t touchId)
{
	for (Pointer &p : state.pointers) {
		if (p.button == Pointer::Touch && p.touchId == touchId) {
			return p;
		}
	}

	Pointer &p = state.pointers.push();
	p.button = Pointer::Touch;
	p.touchId = touchId;
	return p;
}

static Pointer *findTouch(InputState &state, uintptr_t touchId)
{
	for (Pointer &p : state.pointers) {
		if (p.button == Pointer::Touch && p.touchId == touchId) {
			return &p;
		}
	}
	return nullptr;
}

void initPointer(InputState &state, Pointer &pointer, const PointerPosition &pos, Pointer::Action action)
{
	pointer.id = ++state.nextPointerId;
	pointer.time = 0.0f;
	pointer.canTap = true;
	pointer.dragStart = pointer.prev = pointer.current = pointer.start = pos;
	pointer.action = action;
}

void movePointer(InputState &state, Pointer &pointer, const PointerPosition &pos)
{
	sf::Vec2 start = pointer.start.pos * state.dpiResolution;
	sf::Vec2 prev = pointer.current.pos * state.dpiResolution;
	sf::Vec2 next = pos.pos * state.dpiResolution;

	pointer.current = pos;

	if (pointer.button != Pointer::MouseHover) {
		float dist = sf::length(prev - next);
		pointer.dragFactor = sf::min(pointer.dragFactor + sf::min(pointer.time*80.0f + 10.0f, 100.0f) * dist, 1.0f);
		if (pointer.dragFactor >= 0.5f && sf::length(start - next) > 0.05f) {
			pointer.canTap = false;
		}
	}
}

void InputState::update(const InputUpdateArgs &args)
{
	memcpy(prevKeyDown, keyDown, sizeof(keyDown));

	mouseBlocked = args.mouseBlocked;
	keyboardBlocked = args.keyboardBlocked;

	clipToWorld = args.clipToWorld;
	rcpResolution = sf::Vec2(1.0f) / args.resolution;
	dpiResolution = sf::Vec2(args.resolution.x*rcpResolution.y, 1.0f) / args.dpiScale;

	for (uint32_t i = 0; i < pointers.size; i++) {
		Pointer &p = pointers[i];
		p.time += args.dt;

		p.prev = p.current;

		if (p.button == Pointer::MouseHover) {
			if (args.mouseBlocked || p.action != Pointer::Down) {
				pointers.removeOrdered(i);
				i--;
			}
			// NOTE: Hover is always in Down state
		} else if (p.action == Pointer::Down) {
			p.action = Pointer::Hold;
		} else if (p.action == Pointer::Up) {
			pointers.removeOrdered(i);
			i--;
		} else if (p.action == Pointer::Cancel) {
			pointers.removeOrdered(i);
			i--;
		}
	}

	for (const sapp_event &e : args.events) {

		if (e.type == SAPP_EVENTTYPE_MOUSE_DOWN) {
			if (args.mouseBlocked) continue;
			PointerPosition pos = sappToPointerPosition(*this, sf::Vec2(e.mouse_x, e.mouse_y));

			if (args.simulateTouch) {
				if (simulatedTouchId) {
					Pointer *p = findTouch(*this, simulatedTouchId);
					if (p->action != Pointer::Up) {
						p->action = Pointer::Cancel;
					}
				}

				simulatedTouchId = (++nextSimulatedTouchId % 1024) + 1;
				Pointer &p = findOrAddTouch(*this, simulatedTouchId);
				initPointer(*this, p, pos, Pointer::Down);
			} else {
				if (Pointer *p = findButton(*this, Pointer::MouseHover)) {
					pointers.removeSwapPtr(p);
				}

				Pointer::Button button = sappToPointerButton(e.mouse_button);
				Pointer &p = findOrAddButton(*this, button);
				initPointer(*this, p, pos, Pointer::Down);
			}

		} else if (e.type == SAPP_EVENTTYPE_MOUSE_MOVE) {
			PointerPosition pos = sappToPointerPosition(*this, sf::Vec2(e.mouse_x, e.mouse_y));

			if (args.simulateTouch) {
				if (simulatedTouchId) {
					if (Pointer *p = findTouch(*this, simulatedTouchId)) {
						movePointer(*this, *p, pos);
					}
				}
			} else {
				bool foundMouse = false;
				for (Pointer &p : pointers) {
					if (p.button <= Pointer::LastMouse && p.button != Pointer::MouseHover) {
						foundMouse = true;
						movePointer(*this, p, pos);
					}
				}
				if (!foundMouse && !args.mouseBlocked) {
					Pointer &p = findOrAddButton(*this, Pointer::MouseHover);
					initPointer(*this, p, pos, Pointer::Down);
				}
			}

		} else if (e.type == SAPP_EVENTTYPE_MOUSE_UP) {
			PointerPosition pos = sappToPointerPosition(*this, sf::Vec2(e.mouse_x, e.mouse_y));

			if (args.simulateTouch) {
				if (simulatedTouchId) {
					if (Pointer *p = findTouch(*this, simulatedTouchId)) {
						p->action = Pointer::Up;
					}
					simulatedTouchId = 0;
				}
			} else {
				Pointer::Button button = sappToPointerButton(e.mouse_button);
				if (Pointer *p = findButton(*this, button)) {
					p->action = Pointer::Up;
				}
			}

		} else if (e.type == SAPP_EVENTTYPE_MOUSE_LEAVE) {
			for (Pointer &p : pointers) {
				if (p.button <= Pointer::LastMouse) {
					p.action = Pointer::Cancel;
				}
			}

			if (simulatedTouchId) {
				if (Pointer *p = findTouch(*this, simulatedTouchId)) {
					p->action = Pointer::Up;
				}
				simulatedTouchId = 0;
			}
		} else if (e.type == SAPP_EVENTTYPE_TOUCHES_BEGAN) {
			if (args.mouseBlocked) continue;

			for (const sapp_touchpoint &touch : sf::slice(e.touches, e.num_touches)) {
				if (!touch.changed) continue;
				PointerPosition pos = sappToPointerPosition(*this, sf::Vec2(touch.pos_x, touch.pos_y));
				Pointer &p = findOrAddTouch(*this, touch.identifier);
				initPointer(*this, p, pos, Pointer::Down);
			}
		} else if (e.type == SAPP_EVENTTYPE_TOUCHES_MOVED) {
			for (const sapp_touchpoint &touch : sf::slice(e.touches, e.num_touches)) {
				if (!touch.changed) continue;
				PointerPosition pos = sappToPointerPosition(*this, sf::Vec2(touch.pos_x, touch.pos_y));
				if (Pointer *p = findTouch(*this, touch.identifier)) {
					movePointer(*this, *p, pos);
				}
			}
		} else if (e.type == SAPP_EVENTTYPE_TOUCHES_ENDED) {
			for (const sapp_touchpoint &touch : sf::slice(e.touches, e.num_touches)) {
				if (!touch.changed) continue;
				if (Pointer *p = findTouch(*this, touch.identifier)) {
					p->action = Pointer::Up;
				}
			}
		} else if (e.type == SAPP_EVENTTYPE_TOUCHES_CANCELLED) {
			for (const sapp_touchpoint &touch : sf::slice(e.touches, e.num_touches)) {
				if (!touch.changed) continue;
				if (Pointer *p = findTouch(*this, touch.identifier)) {
					p->action = Pointer::Cancel;
				}
			}
		} else if (e.type == SAPP_EVENTTYPE_KEY_DOWN) {
			if (!args.keyboardBlocked) {
				keyDown[e.key_code] = true;
			}
		} else if (e.type == SAPP_EVENTTYPE_KEY_UP) {
			keyDown[e.key_code] = false;
		} else if (e.type == SAPP_EVENTTYPE_UNFOCUSED) {
			sf::memZero(keyDown);
			sf::memZero(prevKeyDown);
			for (Pointer &p : pointers) {
				p.action = Pointer::Cancel;
			}
		}
	}

	for (Pointer &p : pointers) {
		// Resample dragStart for same class (at least for Background)
		if (p.button == Pointer::Touch
			&& (p.action == Pointer::Up || p.action == Pointer::Cancel)) {
			for (Pointer &p2 : pointers) {
				if (p2.button == Pointer::Touch
					&& p2.action == Pointer::Hold) {
					p2.dragStart.worldRay = pointerToWorld(clipToWorld, p2.current.pos);
				}
			}
		}
	}
}

Pointer *InputState::find(uint64_t id)
{
	for (Pointer &pointer : pointers) {
		if (pointer.id == id) return &pointer;
	}
	return nullptr;
}

}
