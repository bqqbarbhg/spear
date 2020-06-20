#pragma once

#include "sf/Array.h"
#include "sf/HashMap.h"
#include "sf/Vector.h"
#include "sf/Symbol.h"
#include "sf/Box.h"

#include "GameComponent.h"

namespace sv {

struct Event;
struct Action;

struct InstancedObject
{
	ObjectId objectId;
	int16_t x, y;
	uint8_t offset[3];
	uint8_t rotation;
};

struct State
{
	sf::HashMap<ObjectId, sv::GameObject> objects;
	sf::HashMap<InstanceId, InstancedObject> instances;
	uint32_t nextObjectId = 1;
	uint32_t nextInstanceId = 2;

	void applyEvent(Event *event);
	bool applyAction(Action *action, sf::Array<sf::Box<Event>> &events, sf::StringBuf &error);
};

}
