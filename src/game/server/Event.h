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
		UpdateChunk,
		UpdateTileType,

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

struct EventMove : EventBase<Event::Move>
{
	EntityId entity;
	sf::Vec2i position;
	sf::Array<Waypoint> waypoints;
};

struct EventSpawn : EventBase<Event::Spawn>
{
	sf::Box<sv::Entity> data;
};

struct EventDestroy : EventBase<Event::Destroy>
{
	EntityId entity;
};

struct EventUpdateChunk : EventBase<Event::UpdateChunk>
{
	sf::Vec2i position;
	MapChunk chunk;
};

struct EventUpdateTileType : EventBase<Event::UpdateTileType>
{
	uint32_t index;
	TileType tileType;
};

}
