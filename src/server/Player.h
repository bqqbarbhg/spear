#pragma once

#include "server/Object.h"
#include "server/Card.h"

#include "sf/Array.h"

namespace sv2 {

struct Player : Object
{
	static constexpr Type ObjectType = Object::Player;
	Player() : Object(ObjectType) { }

	sf::Symbol name;
	sf::Array<Card> cards;
};

}
