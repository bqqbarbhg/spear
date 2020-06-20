#pragma once

#include "GameState.h"

namespace sv {

struct Event
{
	#if SF_DEBUG
		virtual void debugForceVtable() { }
	#endif

	enum Type
	{
		Error,
		Move,
		Spawn,
		Destroy,
		UpdateObject,
		RemoveObject,
		UpdateInstance,
		RemoveInstance,

		Type_Count,
		Type_ForceU32 = 0x7fffffff,
	};

	Type type;

	Event() { }
	Event(Type type) : type(type) { }

	template <typename T> T *as() { return type == T::EventType ? (T*)this : nullptr; }
	template <typename T> const T *as() const { return type == T::EventType ? (T*)this : nullptr; }
};

template <Event::Type SelfType>
struct EventBase : Event
{
	static constexpr Type EventType = SelfType;
	EventBase() : Event(SelfType) { }
};

enum class MoveType : uint32_t
{
	Walk,
	Run,
	Teleport,
};

struct Waypoint
{
	MoveType moveType;
	sf::Vec2i position;
};

struct EventUpdateObject : EventBase<Event::UpdateObject>
{
	ObjectId id;
	sv::GameObject object;
};

struct EventRemoveObject : EventBase<Event::RemoveObject>
{
	ObjectId id;
};

struct EventUpdateInstance : EventBase<Event::UpdateInstance>
{
	InstanceId id;
	sv::InstancedObject instance;
};

struct EventRemoveInstance : EventBase<Event::RemoveInstance>
{
	InstanceId id;
};

}
