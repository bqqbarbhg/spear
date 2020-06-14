#pragma once

#include "sf/HashMap.h"

#include "server/IdMap.h"
#include "server/Entity.h"

namespace sv {

struct Event;

inline sf_forceinline uint64_t getIdImp(sf::Box<Component> &box) { return box ? box->id.id : 0; }

struct DirtyList
{
	sf::HashMap<ComponentId, uint32_t> components;
	sf::HashMap<ProtoId, uint32_t> protos;
	sf::HashMap<EntityId, uint32_t> entities;
};

struct State
{
	IdMap<sf::Box<Component>, ComponentId> components;
	IdMap<Proto, ProtoId> protos;
	IdMap<Entity, EntityId> entities;

	const Component *findComponent(ComponentId id) const;
	const Proto *findProto(ProtoId id) const;
	const Entity *findEntity(EntityId id) const;

	void applyEvent(const Event *event, DirtyList &dirty);
};

}
