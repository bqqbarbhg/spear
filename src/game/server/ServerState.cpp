#include "ServerState.h"

#include "game/server/Event.h"

#include "sf/Reflection.h"

namespace sv {

Handles *t_handles;

void State::applyEvent(const Event &event, sf::Array<sf::Box<Event>> *undoList=nullptr)
{
	if (const EventCreateObject *e = event.as<EventCreateObject>()) {
		retainObject(*this, e->object, +1);
		objects[e->id] = e->object;

		if (undoList) {
			sf::Box<EventRemoveObject> undo = sf::box<EventRemoveObject>();
			undo->id = e->id;
			undoList->push(undo);
		}

	} else if (const EventUpdateObject *e = event.as<EventUpdateObject>()) {
		Object *obj = objects.findValue(e->id);
		sf_assert(obj);

		if (undoList) {
			sf::Box<EventUpdateObject> undo = sf::box<EventUpdateObject>();
			undo->id = e->id;
			undo->object = *obj;
			undoList->push(undo);
		}

		retainObject(*this, e->object, +1);
		retainObject(*this, *obj, -1);

		uint32_t prevRefs = obj->refCount;
		*obj = e->object;
		obj->refCount = prevRefs;

	} else if (const EventRemoveObject *e = event.as<EventRemoveObject>()) {

		Object *obj = objects.findValue(e->id);
		sf_assert(obj);

		if (undoList) {
			sf::Box<EventCreateObject> undo = sf::box<EventCreateObject>();
			undo->id = e->id;
			undo->object = *obj;
			undoList->push(undo);
		}

		retainObject(*this, *obj, -1);

	} else if (const EventCreateInstance *e = event.as<EventCreateInstance>()) {
		retainObjectId(*this, e->instance.objectId, +1);
		instances[e->id] = e->instance;
	} else if (const EventUpdateInstance *e = event.as<EventUpdateInstance>()) {
		Instance *inst = instances.findValue(e->id);
		sf_assert(inst);
		sf_assert(inst->objectId == e->instance.objectId);
		*inst = e->instance;
	} else if (const EventRemoveInstance *e = event.as<EventRemoveInstance>()) {
		Instance *inst = instances.findValue(e->id);
		sf_assert(inst);
		retainObjectId(*this, inst->objectId, -1);
		instances.remove(e->id);
	}
}

}

namespace sf {

template<> void initType<sv::Card>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::Card, typeId),
	};
	sf_struct(t, sv::Card, fields);
}

template<> void initType<sv::Component>(Type *t)
{
	static PolymorphType polys[] = {
		sf_poly(sv::Component, Model, sv::ModelComponent),
		sf_poly(sv::Component, PointLight, sv::PointLightComponent),
		sf_poly(sv::Component, Character, sv::CharacterComponent),
		sf_poly(sv::Component, Card, sv::CardComponent),
	};
	sf_struct_poly(t, sv::Component, type, { }, polys);
}

template<> void initType<sv::ModelComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::ModelComponent, model),
		sf_field(sv::ModelComponent, shadowModel),
		sf_field(sv::ModelComponent, material),
		sf_field(sv::ModelComponent, position),
		sf_field(sv::ModelComponent, rotation),
		sf_field(sv::ModelComponent, scale),
		sf_field(sv::ModelComponent, stretch),
		sf_field(sv::ModelComponent, castShadows),
	};
	sf_struct_base(t, sv::ModelComponent, sv::Component, fields);
}

template<> void initType<sv::PointLightComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::PointLightComponent, color),
		sf_field(sv::PointLightComponent, intensity),
		sf_field(sv::PointLightComponent, radius),
		sf_field(sv::PointLightComponent, position),
	};
	sf_struct_base(t, sv::PointLightComponent, sv::Component, fields);
}

template<> void initType<sv::CharacterComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::CharacterComponent, model),
		sf_field(sv::CharacterComponent, players),
		sf_field(sv::CharacterComponent, cards),
	};
	sf_struct_base(t, sv::CharacterComponent, sv::Component, fields);
}

template<> void initType<sv::CardComponent>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::CardComponent, image),
		sf_field(sv::CardComponent, name),
		sf_field(sv::CardComponent, description),
	};
	sf_struct_base(t, sv::CardComponent, sv::Component, fields);
}

template<> void initType<sv::Object>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::Object, refCount),
		sf_field(sv::Object, path),
		sf_field(sv::Object, name),
		sf_field(sv::Object, components),
	};
	sf_struct(t, sv::Object, fields);
}

template<> void initType<sv::Instance>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::Instance, objectId),
		sf_field(sv::Instance, x),
		sf_field(sv::Instance, y),
		sf_field(sv::Instance, offset),
		sf_field(sv::Instance, rotation),
	};
	sf_struct(t, sv::Instance, fields);
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
