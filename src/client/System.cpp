#include "System.h"

#include "client/AreaSystem.h"
#include "client/LightSystem.h"
#include "client/EnvLightSystem.h"
#include "client/ModelSystem.h"
#include "client/TileModelSystem.h"
#include "client/CharacterModelSystem.h"
#include "client/ParticleSystem.h"
#include "client/BlobShadowSystem.h"
#include "client/GameSystem.h"
#include "client/TapAreaSystem.h"
#include "client/BillboardSystem.h"
#include "client/EffectSystem.h"
#include "client/AudioSystem.h"

#include "sf/Reflection.h"

namespace cl {

void BoneUpdates::clear()
{
	for (sf::Array<BoneUpdates::Update> &group : updates) {
		group.clear();
	}
}

System::~System() { }

void Systems::init(const SystemsDesc &desc)
{
	area = AreaSystem::create();
	light = LightSystem::create();
	envLight = EnvLightSystem::create();
	model = ModelSystem::create();
	tileModel = TileModelSystem::create();
	characterModel = CharacterModelSystem::create(desc);
	particle = ParticleSystem::create(desc);
	blobShadow = BlobShadowSystem::create();
	game = GameSystem::create(desc);
	tapArea = TapAreaSystem::create();
	billboard = BillboardSystem::create();
	effect = EffectSystem::create();
	audio = AudioSystem::create(desc);
}

void Systems::updateVisibility(VisibleAreas &areas, uint32_t areaFlags, const sf::Frustum &frustum)
{
	areas.areas.clear();
	area->queryFrustum(areas.areas, areaFlags, frustum);

	for (sf::Array<uint32_t> &groupIds : areas.groups) {
		groupIds.clear();
	}

	for (const Area &area : areas.areas) {
		if ((uint32_t)area.group >= sf_arraysize(areas.groups)) continue;
		areas.groups[(uint32_t)area.group].push(area.userId);
	}
}

void Systems::renderShadows(const RenderArgs &renderArgs)
{
	updateVisibility(shadowAreas, Area::Shadow, renderArgs.frustum);

	tileModel->renderShadow(shadowAreas, renderArgs);
	characterModel->renderShadow(shadowAreas, renderArgs);
}

void Systems::renderEnvmapGBuffer(const RenderArgs &renderArgs)
{
	updateVisibility(envmapAreas, Area::Envmap, renderArgs.frustum);

	tileModel->renderEnvmapGBuffer(envmapAreas, renderArgs);
	characterModel->renderEnvmapGBuffer(envmapAreas, renderArgs);
}

bool EntitySystem::prepareForRemove(Systems &systems, uint32_t entityId, const EntityComponent &ec, const FrameArgs &args)
{
	return true;
}

void EntitySystem::editorHighlight(Systems &systems, const EntityComponent &ec, EditorHighlight type)
{
	// Nop
}

void Entities::addPrefabComponents(Systems &systems, uint32_t prefabId)
{
	Prefab &prefab = prefabs[prefabId];
	for (const sf::Box<sv::Component> &comp : prefab.svPrefab->components) {
		if (const auto *c = comp->as<sv::ParticleSystemComponent>()) {
			systems.particle->reserveEffectType(systems, comp.cast<sv::ParticleSystemComponent>());
		} else if (const auto *c = comp->as<sv::CharacterModelComponent>()) {
			ComponentData &data = prefab.componentData.push();
			data.component = c;
			data.data = systems.characterModel->preloadCharacterModel(*c);
		} else if (const auto *c = comp->as<sv::DynamicModelComponent>()) {
			ComponentData &data = prefab.componentData.push();
			data.component = c;
			data.data = systems.model->preloadModel(*c);
		} else if (const auto *c = comp->as<sv::SoundComponent>()) {
			ComponentData &data = prefab.componentData.push();
			data.component = c;
			data.data = systems.audio->preloadSound(*c);
		}
	}
}

void Entities::removePrefabComponents(Systems &systems, uint32_t prefabId)
{
	Prefab &prefab = prefabs[prefabId];
	for (const sf::Box<sv::Component> &comp : prefab.svPrefab->components) {
		if (const auto *c = comp->as<sv::ParticleSystemComponent>()) {
			systems.particle->releaseEffectType(systems, comp.cast<sv::ParticleSystemComponent>());
		}
	}
}

uint32_t Entities::addPrefab(Systems &systems, const sv::Prefab &svPrefab)
{
	auto res = nameToPrefab.insert(svPrefab.name);
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

	addPrefabComponents(systems, prefabId);

	return prefabId;
}

void Entities::removePrefab(Systems &systems, uint32_t prefabId)
{
	Prefab &prefab = prefabs[prefabId];
	sf_assert(prefab.entityIds.size == 0);

	removePrefabComponents(systems, prefabId);

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
	Entity &entity = entities[entityId];
	if (entity.deleteQueued) return;
	entity.deleteQueued = true;

	removeQueue.push(entityId);
}

void Entities::addComponents(Systems &systems, uint32_t entityId, const Transform &transform, const Prefab &prefab)
{
	uint8_t compIx = 0;
	for (const sf::Box<sv::Component> &comp : prefab.svPrefab->components) {

		if (const auto *c = comp->as<sv::DynamicModelComponent>()) {
			systems.model->addModel(systems, entityId, compIx, *c, transform);
		} else if (const auto *c = comp->as<sv::TileModelComponent>()) {
			systems.tileModel->addModel(systems, entityId, compIx, *c, transform);
		} else if (const auto *c = comp->as<sv::CharacterModelComponent>()) {
			systems.characterModel->addCharacterModel(systems, entityId, compIx, *c, transform);
		} else if (const auto *c = comp->as<sv::ParticleSystemComponent>()) {
			systems.particle->addEffect(systems, entityId, compIx, comp.cast<sv::ParticleSystemComponent>(), transform);
		} else if (const auto *c = comp->as<sv::PointLightComponent>()) {
			systems.light->addPointLight(systems, entityId, compIx, *c, transform);
		} else if (const auto *c = comp->as<sv::BlobShadowComponent>()) {
			systems.blobShadow->addBlobShadow(systems, entityId, compIx, *c, transform);
		} else if (const auto *c = comp->as<sv::TapAreaComponent>()) {
			systems.tapArea->addTapArea(systems, entityId, compIx, *c, transform);
		} else if (const auto *c = comp->as<sv::SoundComponent>()) {
			systems.audio->addSound(systems, entityId, compIx, *c, transform);
		}

		compIx++;
	}
}

void Entities::removeComponents(Systems &systems, uint32_t entityId)
{
	Entity &entity = entities[entityId];
	uint32_t numComponents = entity.components.size;
	for (uint32_t i = numComponents; i > 0; --i) {
		EntityComponent &ec = entity.components[i - 1];
		ec.system->remove(systems, entityId, ec);
		sf_assert(entity.components.size == numComponents);
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

void Entities::removeComponent(uint32_t entityId, EntitySystem *system, uint32_t userId, uint8_t subsystemIndex)
{
	Entity &entity = entities[entityId];
	for (EntityComponent &ec : entity.components) {
		if (ec.system == system && ec.userId == userId && ec.subsystemIndex == subsystemIndex) {
			entity.components.removeSwapPtr(&ec);
			break;
		}
	}
}

void Entities::updateQueuedRemoves(Systems &systems, const FrameArgs &frameArgs)
{
	for (uint32_t i = 0; i < removeQueue.size; i++) {
		uint32_t entityId = removeQueue[i];
		Entity &entity = entities[entityId];

		for (uint32_t i = 0; i < entity.components.size; i++) {
			EntityComponent &ec = entity.components[i];
			uint32_t prevSize = entity.components.size;
			if ((ec.flags & Entity::PrepareForRemove) == 0 || ec.system->prepareForRemove(systems, entityId, ec, frameArgs)) {
				ec.system->remove(systems, entityId, ec);
				entity.components.removeSwap(i--);
				prevSize--;
			}
			sf_assert(entity.components.size == prevSize);
		}

		if (entity.components.size == 0) {
			removeEntityInstant(systems, entityId);
			removeQueue.removeSwap(i--);
		}
	}
}

}

namespace sf {

template<> void initType<cl::ClientPersist>(Type *t)
{
	static Field fields[] = {
		sf_field(cl::ClientPersist, camera),
		sf_field(cl::ClientPersist, zoom),
	};
	sf_struct(t, cl::ClientPersist, fields);
}

}
