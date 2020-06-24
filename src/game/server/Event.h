#pragma once

#include "ServerState.h"

namespace sv {

struct Event
{
	#if SF_DEBUG
		virtual void debugForceVtable() { }
	#endif

	enum Type
	{
		Error,
		CreateObject,
		UpdateObject,
		RemoveObject,
		CreateInstance,
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

#if 0
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

struct EventMove : EventBase<Event::Move>
{
	EntityId entity;
	sf::Vec2i position;
	sf::Array<Waypoint> waypoints;
};
#endif

struct EventCreateObject : EventBase<Event::CreateObject>
{
	sv::ObjectId id;
	sv::Object object;
};

struct EventUpdateObject : EventBase<Event::UpdateObject>
{
	sv::ObjectId id;
	sv::Object object;
};

struct EventRemoveObject : EventBase<Event::RemoveObject>
{
	sv::ObjectId id;
};

struct EventCreateInstance : EventBase<Event::CreateInstance>
{
	sv::InstanceId id;
	sv::Instance instance;
};

struct EventUpdateInstance : EventBase<Event::UpdateInstance>
{
	sv::InstanceId id;
	sv::Instance instance;
};

struct EventRemoveInstance : EventBase<Event::RemoveInstance>
{
	sv::InstanceId id;
};

}
