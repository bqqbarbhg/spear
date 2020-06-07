#pragma once

#include "server/Entity.h"

namespace sv {

struct Event
{
	#if SF_DEBUG
		virtual void debugForceVtable() { }
	#endif

	enum Type
	{
		Error,

		AddProto,
		RemoveProto,

		AddEntity,
		UpdateEntity,
		RemoveEntity,

		AddComponent,
		UpdateComponent,
		RemoveComponent,

		Type_Count,
		Type_ForceU32 = 0x7fffffff,
	};

	Type type;

	Event() : type(Error) { }
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

struct EventAddProto : EventBase<Event::AddProto>
{
	sv::Proto proto;
};

struct EventRemoveProto : EventBase<Event::RemoveProto>
{
	sv::ProtoId id;
};

struct EventAddEntity : EventBase<Event::AddEntity>
{
	sv::Entity entity;
};

struct EventUpdateEntity : EventBase<Event::UpdateEntity>
{
	sv::Entity entity;
};

struct EventRemoveEntity : EventBase<Event::RemoveEntity>
{
	sv::EntityId id;
};

struct EventAddComponent : EventBase<Event::AddComponent>
{
	sf::Box<sv::Component> component;
};

struct EventUpdateComponent : EventBase<Event::UpdateComponent>
{
	sf::Box<sv::Component> component;
};

struct EventRemoveComponent : EventBase<Event::RemoveComponent>
{
	sv::ComponentId id;
};

}
