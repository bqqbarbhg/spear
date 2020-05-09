#include "ClientState.h"

namespace cl {

sf::Box<Entity> convertEntity(const sf::Box<sv::Entity> &svEntity)
{
	sf::Box<Entity> data;
	if (auto ent = svEntity->as<sv::Character>()) {
		data = sf::box<Character>(svEntity);
	} else {
		sf_failf("Unhandled entity type: %u", svEntity->type);
	}
	return data;
}

void State::reset(sv::State *svState)
{
	entities.clear();
	entities.resize(svState->entities.size);
	uint32_t ix = 0;
	for (sf::Box<sv::Entity> &svEntity : svState->entities) {
		if (svEntity) {
			entities[ix] = convertEntity(svEntity);
		}
		ix++;
	}
}

void State::applyEvent(sv::Event *event)
{
	if (auto e = event->as<sv::EventMove>()) {

		Entity *data = entities[e->entity];

		if (auto d = data->as<Character>()) {

			d->waypoints.push(e->waypoints);

		} else {

			data->position.x = (float)e->position.x;
			data->position.y = 0.0f;
			data->position.z = (float)e->position.y;

		}

	} else if (auto e = event->as<sv::EventSpawn>()) {

		sv::EntityId id = e->data->id;
		while (id >= entities.size) entities.push();
		sf_assert(!entities[id]);
		entities[id] = convertEntity(e->data);

	} else if (auto e = event->as<sv::EventDestroy>()) {

		entities[e->entity].reset();

	} else {
		sf_failf("Unhandled event type: %u", e->type);
	}
}

}
