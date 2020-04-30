#pragma once

#include "sf/Base.h"

struct Entity
{
	uint32_t id;

	sf_forceinline bool operator==(const Entity &rhs) const { return id == rhs.id; }
	sf_forceinline bool operator!=(const Entity &rhs) const { return id != rhs.id; }
};

sf_inline uint32_t hash(Entity e) { return sf::hash(e.id); }
