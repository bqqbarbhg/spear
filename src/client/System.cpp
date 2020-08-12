#include "System.h"

#include "client/AreaSystem.h"
#include "client/ModelSystem.h"
#include "client/CharacterModelSystem.h"
#include "client/ParticleSystem.h"
#include "client/GameSystem.h"

namespace cl {

void Systems::init(const SystemsDesc &desc)
{
	area = AreaSystem::create();
	model = ModelSystem::create();
	characterModel = CharacterModelSystem::create(desc);
	particle = ParticleSystem::create(desc);
	game = GameSystem::create();
}

bool EntitySystem::prepareForRemove(Systems &systems, uint32_t entityId, const EntityComponent &ec)
{
	return true;
}

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
	prefab.svPrefab = sf::box<sv::Prefab>(svPrefab);

	return prefabId;
}

void Entities::removePrefab(uint32_t prefabId)
{
	Prefab &prefab = prefabs[prefabId];
	sf_assert(prefab.entityIds.size == 0);

	sf::reset(prefab);
	freePrefabIds.push(prefabId);
}

sf::Box<sv::Prefab> Entities::findPrefab(const sf::Symbol &name) const
{
	if (const uint32_t *id = nameToPrefab.findValue(name)) {
		return prefabs[*id].svPrefab;
	}
	return { };
}

uint32_t Entities::addEntityImp(uint32_t svId, const Transform &transform, uint32_t prefabId, uint32_t indexInPrefab)
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
		svToEntity.insertDuplicate(svId, entityId);
	}

	return entityId;
}

uint32_t Entities::addEntity(Systems &systems, uint32_t svId, const Transform &transform, const sf::Symbol &prefabName)
{
	uint32_t *pPrefabId = nameToPrefab.findValue(prefabName);
	if (!pPrefabId) return ~0u;
	uint32_t prefabId = *pPrefabId;

	Prefab &prefab = prefabs[prefabId];

	uint32_t entityId = addEntityImp(svId, transform, prefabId, prefab.entityIds.size);
	prefab.entityIds.push(entityId);

	addComponents(systems, entityId, transform, prefab);

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
		ec.system->updateTransform(systems, entityId, ec, update);
	}
}

void Entities::removeEntityInstant(Systems &systems, uint32_t entityId)
{
	removeComponents(systems, entityId);

	Entity &entity = entities[entityId];
	if (entity.prefabId != ~0u) {
		Prefab &prefab = prefabs[entity.prefabId];
		entities[prefab.entityIds.back()].indexInPrefab = entity.indexInPrefab;
		prefab.entityIds.removeSwap(entity.indexInPrefab);
	}
	if (entity.svId) {
		svToEntity.removeExistingPair(entity.svId, entityId);
	}
	freeEntityIds.push(entityId);

	sf::reset(entity);
}

void Entities::removeEntityQueued(uint32_t entityId)
{
	removeQueue.push(entityId);
}

void Entities::addComponents(Systems &systems, uint32_t entityId, const Transform &transform, const Prefab &prefab)
{
	uint8_t compIx = 0;
	for (const sf::Box<sv::Component> &comp : prefab.svPrefab->components) {

		if (const auto *c = comp->as<sv::ModelComponent>()) {
			systems.model->addModel(systems, entityId, compIx, *c, transform);
		} else if (const auto *c = comp->as<sv::CharacterModelComponent>()) {
			systems.characterModel->addCharacterModel(systems, entityId, compIx, *c, transform);
		} else if (const auto *c = comp->as<sv::ParticleSystemComponent>()) {
			systems.particle->addEffect(systems, entityId, compIx, comp.cast<sv::ParticleSystemComponent>(), transform);
		}

		compIx++;
	}
}

void Entities::removeComponents(Systems &systems, uint32_t entityId)
{
	Entity &entity = entities[entityId];
	for (uint32_t i = entity.components.size; i > 0; --i) {
		EntityComponent &ec = entity.components[i - 1];
		ec.system->remove(systems, entityId, ec);
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

void Entities::updateQueuedRemoves(Systems &systems)
{
	for (uint32_t i = 0; i < removeQueue.size; i++) {
		uint32_t entityId = removeQueue[i];
		Entity &entity = entities[entityId];

		bool allDone = true;
		for (EntityComponent &ec : entity.components) {
			if ((ec.flags & Entity::PrepareForRemove) == 0) continue;
			if (!ec.system->prepareForRemove(systems, entityId, ec)) {
				allDone = false;
			}
		}

		if (allDone) {
			removeEntityInstant(systems, entityId);
			removeQueue.removeSwap(i--);
		}
	}
}

}
