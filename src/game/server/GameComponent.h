#pragma once

#include "sf/Vector.h"
#include "sf/Quaternion.h"
#include "sf/Symbol.h"
#include "sf/Array.h"
#include "sf/Box.h"

namespace sv {

typedef uint32_t ObjectId;
typedef uint32_t InstanceId;

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
		Character,
		Card,

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

struct ModelComponent : ComponentBase<Component::Model>
{
	sf::Symbol model;
	sf::Symbol shadowModel;
	sf::Symbol material;
	sf::Vec3 position;
	sf::Vec3 rotation;
	float scale = 1.0f;
	sf::Vec3 stretch = sf::Vec3(1.0f);
	bool castShadows = true;
};

struct PointLightComponent : ComponentBase<Component::PointLight>
{
	sf::Vec3 color = sf::Vec3(1.0f);
	float intensity = 1.0f;
	float radius = 1.0f;
	sf::Vec3 position;
};

struct Card
{
	ObjectId type;
};

struct CharacterComponent : ComponentBase<Component::Character>
{
	sf::Symbol model;
	sf::Array<uint32_t> players;
	sf::Array<sv::Card> cards;
};

struct CardComponent : ComponentBase<Component::Card>
{
	sf::Symbol image;
	sf::Symbol name;
	sf::Array<sf::Symbol> description;
};

struct GameObject
{
	sf::Symbol id;
	sf::Symbol name;
	sf::Array<sf::Box<Component>> components;
};

}
