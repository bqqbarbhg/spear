#include "ServerState.h"

#include "server/Event.h"

namespace sv {

const Component *State::findComponent(ComponentId id) const
{
	const sf::Box<Component> *box = components.find(id);
	return box ? box->ptr : nullptr;
}

const Proto *State::findProto(ProtoId id) const
{
	return protos.find(id);
}

const Entity *State::findEntity(EntityId id) const
{
	return entities.find(id);
}

void State::applyEvent(const Event *event, DirtyList &dirty)
{
	switch (event->type) {

	case Event::AddProto: {
		EventAddProto *e = (EventAddProto*)event;
		sf::InsertResult<Proto> res = protos.insertUninit(e->proto.id);
		sf_assert(res.inserted);
		new (&res.entry) Proto(e->proto);

		for (const sf::Box<sv::Component> &comp : e->proto.components) {
			sf_assert(comp->protoId == e->proto.id);
			sf::InsertResult<sf::Box<Component>> cres = components.insertUninit(comp->id);
			sf_assert(cres.inserted);
			cres.entry = comp;
		}

	} break;

	case Event::RemoveProto: {
		EventRemoveProto *e = (EventRemoveProto*)event;
		bool removed = protos.remove(e->id);
		(void)removed;
		sf_assert(removed);
	} break;

	case Event::AddEntity: {
		EventAddEntity *e = (EventAddEntity*)event;
		sf::InsertResult<Entity> res = entities.insertUninit(e->entity.id);
		sf_assert(res.inserted);
		new (&res.entry) Entity(e->entity);
		uint32_t flags = D_Added;
		Proto *proto = protos.find(e->entity.protoId);
		proto->entities.push(e->entity.id);
		sf_assert(proto);
		for (const Component *c : proto->components) {
			flags |= c->dirtyFlags;
		}
		dirty.entities[e->entity.id] |= flags;
	} break;

	case Event::UpdateEntity: {
		EventUpdateEntity *e = (EventUpdateEntity*)event;
		Entity *entity = entities.find(e->entity.id);
		sf_assert(entity->protoId == e->entity.protoId);
		sf_assert(entity);
		uint32_t flags = D_Updated;
		if (entity->x != e->entity.x || entity->y != e->entity.y || entity->rotation != e->entity.rotation) {
			flags |= D_Position;
		}
		dirty.entities[e->entity.id] |= flags;

		*entity = e->entity;
	} break;

	case Event::RemoveEntity: {
		EventRemoveEntity *e = (EventRemoveEntity*)event;
		bool removed = entities.remove(e->id);
		(void)removed;
		sf_assert(removed);
		dirty.entities[e->id] |= D_Removed;
	} break;

	case Event::UpdateComponent: {
		EventUpdateComponent *e = (EventUpdateComponent*)event;
		sf_assert(e->component);
		const sf::Box<Component> *box = components.find(e->component->id);
		sf_assert(box);
		Component *component = *box;
		sf_assert(component);
		sf_assert(component->type == e->component->type);
		*component = *e->component;
	} break;

	}
}

}

