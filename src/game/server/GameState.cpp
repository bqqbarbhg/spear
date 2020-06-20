#include "GameState.h"

#include "sf/Reflection.h"
#include "game/server/Event.h"
#include "game/server/Action.h"

#include "sf/Mutex.h"

namespace sv {

void State::applyEvent(Event *event)
{
	if (auto e = event->as<sv::EventUpdateObject>()) {
		objects[e->id] = e->object;
	} else if (auto e = event->as<sv::EventRemoveObject>()) {
		objects.remove(e->id);
	} else if (auto e = event->as<sv::EventUpdateInstance>()) {
		instances[e->id] = e->instance;
	} else if (auto e = event->as<sv::EventRemoveInstance>()) {
		instances.remove(e->id);
	} else {
		sf_failf("Unhandled event type: %u", event->type);
	}
}

bool State::applyAction(Action *action, sf::Array<sf::Box<Event>> &events, sf::StringBuf &error)
{
	if (auto move = action->as<ActionMove>()) {

#if 0
		if (!canStandOn(move->position)) {
			error.format("Tile (%d,%d) is not free", move->position.x, move->position.y);
			return false;
		}

		auto event = sf::box<EventMove>();
		event->entity = move->entity;
		event->position = move->position;
		events.push(event);
#endif

		return true;

	} else {
		sf_failf("Unexpected action type: %u", action->type);
		return false;
	}
}

}

namespace sf {

template<> void initType<sv::InstancedObject>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::InstancedObject, objectId),
		sf_field(sv::InstancedObject, x),
		sf_field(sv::InstancedObject, y),
		sf_field(sv::InstancedObject, offset),
		sf_field(sv::InstancedObject, rotation),
	};
	sf_struct(t, sv::InstancedObject, fields, sf::Type::CompactString);
}

template<> void initType<sv::State>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::State, objects),
		sf_field(sv::State, instances),
		sf_field(sv::State, nextObjectId),
		sf_field(sv::State, nextInstanceId),
	};
	sf_struct(t, sv::State, fields);
}


}
