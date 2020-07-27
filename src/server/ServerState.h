#pragma once

#include "sf/HashMap.h"
#include "sf/Box.h"
#include "sf/Symbol.h"

namespace sv {

using ComponentId = uint32_t;

struct Component
{
	sf::Symbol path;

	#if SF_DEBUG
		virtual void debugForceVtable() { }
	#endif

	enum Type
	{
		Error,
		Model,
		PointLight,
		ParticleSystem,

		Type_Count,
		Type_ForceU32 = 0x7fffffff,
	};

	Type type;

	Component() { }
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

struct ServerState
{
	sf::HashMap<ComponentId, sf::Box<Component>> components;
};

}
