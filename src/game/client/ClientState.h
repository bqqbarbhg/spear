#pragma once

#include "game/server/GameState.h"
#include "game/server/Event.h"

namespace cl {

struct Entity
{
	using Type = sv::Entity::Type;

	sv::Entity::Type type;
	sf::Box<sv::Entity> svEntity;
	sf::Vec2 position;

	Entity() { }
	Entity(Type type, sf::Box<sv::Entity> svEntity) : type(type), svEntity(std::move(svEntity)) { }

	template <typename T> T *as() { return type == T::EntityType ? (T*)this : nullptr; }
	template <typename T> const T *as() const { return type == T::EntityType ? (T*)this : nullptr; }
};

template <Entity::Type SelfType>
struct EntityBase : Entity
{
	static constexpr Type EntityType = SelfType;
	EntityBase(sf::Box<sv::Entity> svEntity) : Entity(SelfType, svEntity) { }
};

struct Character : EntityBase<sv::Entity::Character>
{
	Character(sf::Box<sv::Entity> svEntity) : EntityBase(std::move(svEntity)) { }

	sf::Array<sv::Waypoint> waypoints;
};

struct State
{
	sf::Array<sf::Box<Entity>> entities;

	sf::Vec3 getWorldPosition(const sf::Vec2 &tile);
	sf::Vec3 getWorldPosition(const sf::Vec2i &tile);

	void reset(sv::State *svState);

	void applyEvent(sv::Event *event);
};

}
