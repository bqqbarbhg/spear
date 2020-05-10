#include "ClientState.h"

namespace cl {

sf::Box<Entity> convertEntity(const sf::Box<sv::Entity> &svEntity)
{
	sf::Box<Entity> data;
	if (auto ent = svEntity->as<sv::Character>()) {
		auto chr = sf::box<Character>(svEntity);
		data = chr;

		chr->model.load(ent->model);

	} else {
		sf_failf("Unhandled entity type: %u", svEntity->type);
	}

	data->position = sf::Vec2(svEntity->position);

	return data;
}

void State::reset(sv::State *svState)
{
	entities.clear();
	entities.resize(svState->entities.size);

	{
		uint32_t ix = 0;
		for (sf::Box<sv::Entity> &svEntity : svState->entities) {
			if (svEntity) {
				entities[ix] = convertEntity(svEntity);
			}
			ix++;
		}
	}

	tileTypes.clear();
	tileTypes.reserve(svState->map.tileTypes.size);

	{
		for (sv::TileType &tileType : svState->map.tileTypes) {
			cl::TileType &dst = tileTypes.push();
			if (tileType.name) {
				dst.tile.load(tileType.name);
			}
		}
	}
}

void State::applyEvent(sv::Event *event)
{
	if (auto e = event->as<sv::EventMove>()) {

		Entity *data = entities[e->entity];

		if (auto d = data->as<Character>()) {

			if (e->waypoints.size) {
				d->waypoints.push(e->waypoints);
			} else {
				d->waypoints.clear();
				data->position = sf::Vec2(e->position);
			}

		} else {
			data->position = sf::Vec2(e->position);
		}

	} else if (auto e = event->as<sv::EventSpawn>()) {

		sv::EntityId id = e->data->id;
		sf_assert(id != 0);
		while (id >= entities.size) entities.push();
		sf_assert(!entities[id]);
		sf::Box<Entity> entity = convertEntity(e->data);
		entity->position = sf::Vec2(e->data->position);
		entities[id] = entity;

	} else if (auto e = event->as<sv::EventDestroy>()) {

		entities[e->entity].reset();

	} else {
		sf_failf("Unhandled event type: %u", e->type);
	}
}

}
