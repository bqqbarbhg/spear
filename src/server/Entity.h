#pragma once

#include <stdint.h>
#include "sf/Symbol.h"
#include "sf/Array.h"
#include "sf/Box.h"

namespace sv {

enum DirtyFlags
{
	D_Position = 0x1,
	D_TileChunk = 0x2,
};

struct EntityId
{
	uint64_t id;
	EntityId() : id(0) { }
	explicit EntityId(uint64_t id) : id(id) { }
	sf_forceinline bool operator==(const EntityId &rhs) const { return id == rhs.id; }
	sf_forceinline bool operator!=(const EntityId &rhs) const { return id == rhs.id; }
};

struct ProtoId
{
	uint64_t id;
	ProtoId() : id(0) { }
	explicit ProtoId(uint64_t id) : id(id) { }
	sf_forceinline bool operator==(const ProtoId &rhs) const { return id == rhs.id; }
	sf_forceinline bool operator!=(const ProtoId &rhs) const { return id == rhs.id; }
};

struct ComponentId
{
	uint64_t id;
	ComponentId() : id(0) { }
	explicit ComponentId(uint64_t id) : id(id) { }
	sf_forceinline bool operator==(const ComponentId &rhs) const { return id == rhs.id; }
	sf_forceinline bool operator!=(const ComponentId &rhs) const { return id == rhs.id; }
};

struct Component
{
	#if SF_DEBUG
		virtual void debugForceVtable() { }
	#endif

	enum Type
	{
		Error,
		Model,
		PointLight,
		TileArea,

		Type_Count,
		Type_ForceU32 = 0x7fffffff,
	};

	ComponentId id;
	ProtoId protoId;
	Type type;
	uint32_t dirtyFlags = 0;

	Component() : type(Error) { }
	Component(Type type) : type(type) { }

	template <typename T> T *as() { return type == T::ComponentType ? (T*)this : nullptr; }
	template <typename T> const T *as() const { return type == T::ComponentType ? (T*)this : nullptr; }
};

template <Component::Type SelfType>
struct ComponentBase : Component
{
	static constexpr Type ComponentType = SelfType;
	ComponentBase() : Component(SelfType) { }
};

struct Proto
{
	ProtoId id;
	sf::Symbol name;
	sf::Array<sf::Box<Component>> components;
	sf::Array<EntityId> entities;
};

struct Entity
{
	EntityId id;
	ProtoId protoId;
	int16_t x, y;
	uint8_t offset[3];
	uint8_t rotation;
};

}
