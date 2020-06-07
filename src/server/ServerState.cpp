#include "ServerState.h"

#include "server/Event.h"

namespace sv {

const Component *State::findComponent(ComponentId id) const
{
	const sf::Box<Component> *box = components.find(id);
	return box ? *box : nullptr;
}

const Proto *State::findProto(ProtoId id) const
{
	return protos.find(id);
}

const Entity *State::findEntity(EntityId id) const
{
	return entities.find(id);
}

void State::applyEvent(const Event *event, DirtyList &list)
{
	switch (event->type) {

	case Event::AddProto: {
		EventAddProto *e = (EventAddProto*)event;
		sf::InsertResult<Proto> res = protos.insertUninit(e->proto.id);
		sf_assert(res.inserted);
		new (&res.entry) Proto(e->proto);
		list.dirtyProtos.insert(e->proto.id);

		for (const sf::Box<sv::Component> &comp : e->proto.components) {
			sf_assert(comp->protoId == e->proto.id);
			list.dirtyProtos.insert(comp->protoId);
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
	} break;

	case Event::UpdateEntity: {
		EventUpdateEntity *e = (EventUpdateEntity*)event;
		Entity *entity = entities.find(e->entity.id);
		sf_assert(entity->protoId == e->entity.protoId);
		sf_assert(entity);
		*entity = e->entity;
	} break;

	case Event::RemoveEntity: {
		EventRemoveEntity *e = (EventRemoveEntity*)event;
		bool removed = entities.remove(e->id);
		(void)removed;
		sf_assert(removed);
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
