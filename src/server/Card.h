#pragma once

#include "server/Object.h"
#include "sf/Box.h"
#include "sf/Symbol.h"

namespace sv2 {

struct CardType : Object
{
	static constexpr Type ObjectType = Object::CardType;
	CardType() : Object(ObjectType) { }

	sf::Symbol name;
	sf::Symbol description;
};

struct Card
{
	sf::Box<CardType> cardType;
	uint8_t cooldownTurns = 0;
};

}
