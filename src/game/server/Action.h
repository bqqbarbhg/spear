#pragma once

#include "game/server/GameState.h"

namespace sv {

struct Action
{
	enum Type
	{
		Error,
		Move,

		Type_Count,
		Type_ForceU32 = 0x7fffffff,
	};

	uint32_t objectId;
	Type type;

	template <typename T> T *as() { return type == T::ActionType ? (T*)this : nullptr; }
	template <typename T> const T *as() const { return type == T::ActionType ? (T*)this : nullptr; }

	Action() : type(Error) { }
	Action(Type type) : type(type) { }
};

template <Action::Type SelfType>
struct ActionBase : Action
{
	static constexpr Type ActionType = SelfType;
	ActionBase() : Action(SelfType) { }
};

struct ActionMove : ActionBase<Action::Move>
{
	sf::Vec2i position;
};

}
