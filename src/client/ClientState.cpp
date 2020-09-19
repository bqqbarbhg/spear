#include "ClientState.h"

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

#include "sf/Frustum.h"

#include "sp/Renderer.h"

#ifdef __APPLE__
	#include <TargetConditionals.h>
	#if TARGET_OS_IPHONE
		#define CL_USE_DEPTH_PREPASS 0
	#endif
#endif

#ifndef CL_USE_DEPTH_PREPASS
#define CL_USE_DEPTH_PREPASS 1
#endif

namespace cl {

static Transform getPropTransform(const sv::PropTransform &transform)
{
	Transform ret;
	ret.position = sf::Vec3((float)transform.position.x, (float)transform.offsetY, (float)transform.position.y) * (1.0f/65536.0f);
	ret.rotation = sf::eulerAnglesToQuat(sf::Vec3(0.0f, (float)transform.rotation, 0.0f) * (sf::F_PI/180.0f/64.0f), sf::EulerOrder::YZX);
	ret.scale = (float)transform.scale * (1.0f/256.0f);
	return ret;
}


static void removeEntities(Systems &systems, uint32_t svId)
{
	sf::SmallArray<uint32_t, 64> entityIds;

	{
		uint32_t entityId;
		sf::UintFind find = systems.entities.svToEntity.findAll(svId);
		while (find.next(entityId)) {
			entityIds.push(entityId);
		}
	}

	for (uint32_t entityId : entityIds) {
		systems.entities.removeEntityQueued(entityId);
	}
}

ClientState::ClientState(const SystemsDesc &desc)
{
	systems.init(desc);
}

static void applyEventImp(ClientState &cs, Systems &systems, const sv::Event &event, bool immediate)
{
	if (const auto *e = event.as<sv::LoadPrefabEvent>()) {
		systems.entities.addPrefab(systems, e->prefab);
	} else if (const auto *e = event.as<sv::AddPropEvent>()) {
		Transform transform = getPropTransform(e->prop.transform);
		systems.entities.addEntity(systems, e->prop.id, transform, e->prop.prefabName);
	} else if (const auto *e = event.as<sv::ReplaceLocalPropEvent>()) {
		if (cs.localClientId == e->clientId) {
			removeEntities(systems, e->localId);
		}
		Transform transform = getPropTransform(e->prop.transform);
		systems.entities.addEntity(systems, e->prop.id, transform, e->prop.prefabName);
	} else if (const auto *e = event.as<sv::MovePropEvent>()) {
		Transform transform = getPropTransform(e->transform);
		sf::UintFind find = systems.entities.svToEntity.findAll(e->propId);
		uint32_t entityId;
		while (find.next(entityId)) {
			systems.entities.updateTransform(systems, entityId, transform);
		}
	} else if (const auto *e = event.as<sv::RemovePropEvent>()) {
		removeEntities(systems, e->propId);
	} else if (const auto *e = event.as<sv::ReloadPrefabEvent>()) {
		if (uint32_t *pPrefabId = systems.entities.nameToPrefab.findValue(e->prefab.name)) {
			Prefab &prefab = systems.entities.prefabs[*pPrefabId];

			systems.entities.removePrefabComponents(systems, *pPrefabId);
			prefab.svPrefab = sf::box<sv::Prefab>(e->prefab);
			systems.entities.addPrefabComponents(systems, *pPrefabId);

			for (uint32_t entityId : prefab.entityIds) {
				Transform transform = systems.entities.entities[entityId].transform;
				systems.entities.removeComponents(systems, entityId);
				systems.entities.addComponents(systems, entityId, transform, prefab);
			}
		} else {
			systems.entities.addPrefab(systems, e->prefab);
		}
	}
}

void ClientState::applyEventImmediate(const sv::Event &event)
{
	applyEventImp(*this, systems, event, true);
	systems.game->applyEventImmediate(systems, event);
}

void ClientState::applyEventQueued(const sf::Box<sv::Event> &event)
{
	applyEventImp(*this, systems, *event, false);
	systems.game->applyEventQueued(systems, event);
}

void ClientState::writePersist(ClientPersist &persist)
{
	systems.game->writePersist(systems, persist);
}

void ClientState::editorPick(sf::Array<EntityHit> &hits, const sf::Ray &ray) const
{
	sf::FastRay fastRay { ray };

	sf::Array<Area> areas;
	systems.area->castRay(areas, Area::EditorPick, fastRay);

	for (Area &area : areas) {
		switch (area.group)
		{
		case AreaGroup::DynamicModel: systems.model->editorPick(hits, fastRay, area.userId); break;
		case AreaGroup::TileChunkCulling: systems.tileModel->editorPick(hits, fastRay, area.userId); break;
		case AreaGroup::TapArea: systems.tapArea->editorPick(hits, fastRay, area.userId); break;
		default:
			sf_failf("Unhandled EditorPick group: %u", area.group);
			break;
		}
	}
}

void ClientState::editorHighlight(uint32_t entityId, EditorHighlight type)
{
	Entity &entity = systems.entities.entities[entityId];
	for (const EntityComponent &ec : entity.components) {
		ec.system->editorHighlight(systems, ec, type);
	}
}

void ClientState::updateCamera(FrameArgs &frameArgs)
{
	systems.frameArgs = frameArgs;
	systems.game->updateCamera(frameArgs);
	systems.audio->updateSpatialSounds(frameArgs.mainRenderArgs.worldToView);
}

void ClientState::update(const sv::ServerState *svState, const FrameArgs &frameArgs)
{
	systems.frameArgs = frameArgs;

	systems.boneUpdates.clear();

	systems.tileModel->garbageCollectChunks(systems.area, frameArgs);
	systems.entities.updateQueuedRemoves(systems, frameArgs);

	if (svState) {
		systems.game->update(*svState, systems, frameArgs);
	}

	systems.model->updateLoadQueue(systems.area);
	systems.tileModel->updateLoadQueue(systems.area);
	systems.characterModel->updateLoadQueue(systems.area);

	systems.light->updateLightFade(frameArgs);

	systems.updateVisibility(systems.activeAreas, Area::Activate, frameArgs.mainRenderArgs.frustum);

	systems.tileModel->uploadVisibleChunks(systems.activeAreas, systems.area, frameArgs);
	systems.particle->updateParticles(systems.activeAreas, frameArgs);
	systems.characterModel->updateAnimations(systems.activeAreas, frameArgs.dt);
	systems.characterModel->updateBoneListeners(systems.boneUpdates, systems.activeAreas);
	systems.characterModel->updateAttachedEntities(systems);

	systems.effect->update(systems.entities, frameArgs);
	systems.audio->update();

	systems.blobShadow->updatePositions(systems.area, systems.boneUpdates, frameArgs);

	systems.updateVisibility(systems.visibleAreas, Area::Visibility, frameArgs.mainRenderArgs.frustum);

	systems.area->optimize();
}

void ClientState::renderShadows()
{
	systems.light->renderShadowMaps(systems, systems.visibleAreas, systems.frameArgs.frameIndex);
	for (uint32_t i = 0; i < 1; i++) {
		systems.envLight->renderEnvmap(systems);
	}

	if (systems.game->getVisualizeGI()) {
		systems.envLight->renderEnvmapDebug(systems, systems.frameArgs.mainRenderArgs);
	}
}

void ClientState::renderMain(const RenderArgs &args)
{
	if (systems.game->getVisualizeGI()) {
		systems.envLight->compositeEnvmapDebug();
		return;
	}

	{
		EnvVisualizeSphereOpts envSphereOpts;
		if (systems.game->getVisualizeGISpheres(envSphereOpts)) {
			systems.envLight->renderEnvmapDebugSpheres(systems.light, args, envSphereOpts);
		}
	}

	#if CL_USE_DEPTH_PREPASS
		systems.tileModel->renderDepthPrepass(systems.visibleAreas, args);
	#endif

	systems.model->renderMain(systems.light, systems.envLight, systems.visibleAreas, args);
	systems.characterModel->renderMain(systems.light, systems.envLight, systems.visibleAreas, args, systems.frameArgs);
	systems.tileModel->renderMain(systems.light,  systems.envLight, systems.visibleAreas, args);
	systems.blobShadow->renderMain(systems.visibleAreas, args);
	systems.billboard->renderMain(systems.visibleAreas, args);
	systems.particle->renderMain(systems.visibleAreas, args);
}

void ClientState::handleGui(const GuiArgs &guiArgs)
{
	systems.game->handleGui(systems, guiArgs);
}

}

