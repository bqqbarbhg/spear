#include "System.h"

namespace cl {

void EntitySystem::editorHighlight(Systems &systems, const EntityComponent &ec, EditorHighlight type)
{
	// Nop
}

uint32_t Entities::addPrefab(const sv::Prefab &svPrefab)
{
	auto &res = nameToPrefab.insert(svPrefab.name);
	if (!res.inserted) return res.entry.val;
	
	uint32_t prefabId = prefabs.size;
	if (freePrefabIds.size > 0) {
		prefabId = freePrefabIds.popValue();
	} else {
		prefabs.push();
	}

	res.entry.val = prefabId;

	Prefab &prefab = prefabs[prefabId];
	prefab.s = svPrefab;

	return prefabId;
}

void Entities::removePrefab(uint32_t prefabId)
{
	Prefab &prefab = prefabs[prefabId];
	sf_assert(prefab.entityIds.size == 0);

	sf::reset(prefab);
	freePrefabIds.push(prefabId);
}

uint32_t Entities::addEntity(uint32_t svId, const Transform &transform, uint32_t prefabId, uint32_t indexInPrefab)
{
	uint32_t entityId = entities.size;
	if (freeEntityIds.size > 0) {
		entityId = freeEntityIds.popValue();
	} else {
		entities.push();
	}

	Entity &entity = entities[entityId];
	entity.svId = svId;
	entity.transform = transform;
	entity.prefabId = prefabId;
	entity.indexInPrefab = indexInPrefab;

	if (svId) {
		svToEntity.insert(svId, entityId);
	}

	return entityId;
}

void Entities::updateTransform(Systems &systems, uint32_t entityId, const Transform &transform)
{
	Entity &entity = entities[entityId];

	TransformUpdate update;
	update.previousTransform = entity.transform;
	update.transform = transform;
	update.entityToWorld = transform.asMatrix();

	entity.transform = transform;

	for (EntityComponent &ec : entity.components) {
		if ((ec.flags & Entity::UpdateTransform) == 0) continue;
		ec.system->updateTransform(systems, ec, update);
	}
}

void Entities::removeEntity(Systems &systems, uint32_t entityId)
{
	removeComponents(systems, entityId);

	Entity &entity = entities[entityId];
	if (entity.prefabId != ~0u) {
		Prefab &prefab = prefabs[entity.prefabId];
		entities[prefab.entityIds.back()].indexInPrefab = entity.indexInPrefab;
		prefab.entityIds.removeSwap(entity.indexInPrefab);
	}
	if (entity.svId) {
		svToEntity.removePair(entity.svId, entityId);
	}
	freeEntityIds.push(entityId);

	sf::reset(entity);
}

void Entities::removeComponents(Systems &systems, uint32_t entityId)
{
	Entity &entity = entities[entityId];
	for (uint32_t i = entity.components.size; i > 0; --i) {
		EntityComponent &ec = entity.components[i - 1];
		ec.system->remove(systems, ec);
	}
	entity.components.clear();
}

void Entities::addComponent(uint32_t entityId, EntitySystem *system, uint32_t userId, uint8_t subsystemIndex, uint8_t componentIndex, uint32_t flags)
{
	Entity &entity = entities[entityId];
	EntityComponent &comp = entity.components.push();
	comp.system = system;
	comp.userId = userId;
	comp.subsystemIndex = subsystemIndex;
	comp.componentIndex = componentIndex;
	comp.flags = (uint16_t)flags;
}

}
