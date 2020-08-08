#include "MeshState.h"

#include "client/ClientGlobal.h"
#include "client/ClientState.h"
#include "client/AreaState.h"

#include "sp/Model.h"
#include "sf/UintMap.h"

// TEMP
#include "game/shader/GameShaders.h"
#include "game/shader/DebugMesh.h"

namespace cl {

struct PendingMesh
{
	uint32_t entityId;
	sp::ModelRef model;
	sp::ModelRef shadowModel;
	sf::Box<sv::ModelComponent> component;
};

struct MeshImp
{
	uint32_t entityId;
	sp::ModelRef model;
	sp::ModelRef shadowModel;
	sf::Bounds3 bounds;
	sf::Mat34 localTransform;
	sf::Mat34 worldTransform;
};

struct MeshStateImp : MeshState
{
	sf::Array<PendingMesh> pendingMeshes;

	sf::Array<MeshImp> meshes;
	sf::Array<uint32_t> freeMeshIndices;

	sf::UintMap entityToMesh;

	sf_forceinline uint32_t allocateMeshId()
	{
		if (freeMeshIndices.size > 0) {
			return freeMeshIndices.popValue();
		} else {
			meshes.push();
			return meshes.size - 1;
		}
	}
};

sf::Box<MeshState> MeshState::create()
{
	return sf::box<MeshStateImp>();
}

static void updateMeshWorldTransform(AreaState *areaState, MeshImp &mesh, const sf::Mat34 &entityTransform)
{
	mesh.worldTransform = entityTransform * mesh.localTransform;
	sf::Bounds3 bounds = sf::transformBounds(mesh.worldTransform, mesh.bounds);
}

static sf::Mat34 getModelComponentTransform(const sv::ModelComponent &c)
{
	return sf::mat::translate(c.position) * (
	sf::mat::rotateZ(c.rotation.z * (sf::F_PI/180.0f)) *
	sf::mat::rotateY(c.rotation.y * (sf::F_PI/180.0f)) *
	sf::mat::rotateX(c.rotation.x * (sf::F_PI/180.0f)) *
	sf::mat::scale(c.stretch * c.scale * 0.01f));
}

void MeshState::addMesh(uint32_t entityId, const sf::Box<sv::ModelComponent> &c)
{
	MeshStateImp *imp = (MeshStateImp*)this;

	clientGlobalState->registerEntity(entityId, Entity::Mesh);

	PendingMesh &pendingMesh = imp->pendingMeshes.push();
	pendingMesh.entityId = entityId;
	pendingMesh.model.load(c->model);
	if (c->castShadows) {
		if (c->shadowModel) {
			pendingMesh.shadowModel.load(c->shadowModel);
		} else {
			pendingMesh.shadowModel = pendingMesh.model;
		}
	}
	pendingMesh.component = c;
}

void MeshState::removeEntity(uint32_t entityId)
{
	MeshStateImp *imp = (MeshStateImp*)this;

	AreaState *areaState = clientGlobalState->areaState;

	sf::UintFind find = imp->entityToMesh.findAll(entityId);
	uint32_t meshId;
	while (find.next(meshId)) {
		areaState->removeWorldBox(AreaMesh, meshId);
		imp->freeMeshIndices.push(meshId);
	}

	for (uint32_t i = 0; i < imp->pendingMeshes.size; i++) {
		PendingMesh &pendingMesh = imp->pendingMeshes[i];
		if (pendingMesh.entityId == entityId) {
			imp->pendingMeshes.removeSwap(i--);
		}
	}
}

void MeshState::updateEntityTransform(uint32_t entityId, const VisualTransform &transform, const sf::Mat34 &matrix)
{
	MeshStateImp *imp = (MeshStateImp*)this;

	AreaState *areaState = clientGlobalState->areaState;

	sf::UintFind find = imp->entityToMesh.findAll(entityId);
	uint32_t meshId;
	while (find.next(meshId)) {
		MeshImp &mesh = imp->meshes[meshId];

		mesh.worldTransform = matrix * mesh.localTransform;

		sf::Bounds3 bounds = sf::transformBounds(mesh.worldTransform, mesh.bounds);
		areaState->updateWorldBox(AreaMesh, meshId, bounds);
	}
}

void MeshState::updatePendingMeshes()
{
	MeshStateImp *imp = (MeshStateImp*)this;

	AreaState *areaState = clientGlobalState->areaState;

	for (uint32_t i = 0; i < imp->pendingMeshes.size; i++) {
		PendingMesh &pendingMesh = imp->pendingMeshes[i];
		if (pendingMesh.model.isLoading() || (!pendingMesh.shadowModel || pendingMesh.shadowModel.isLoading())) continue;

		if (pendingMesh.model.isLoaded() && (!pendingMesh.shadowModel || pendingMesh.shadowModel.isLoaded())) {
			uint32_t entityId = pendingMesh.entityId;
			uint32_t meshId = imp->allocateMeshId();
			MeshImp &mesh = imp->meshes[meshId];
			mesh.entityId = entityId;
			mesh.model = pendingMesh.model;
			mesh.shadowModel = pendingMesh.shadowModel;
			imp->entityToMesh.insert(entityId, meshId);

			sf::Vec3 aabbMin = sf::Vec3(+HUGE_VALF);
			sf::Vec3 aabbMax = sf::Vec3(-HUGE_VALF);
			for (sp::Mesh &mesh : pendingMesh.model->meshes) {
				aabbMin = sf::min(aabbMin, mesh.bounds.origin - mesh.bounds.extent);
				aabbMax = sf::max(aabbMax, mesh.bounds.origin + mesh.bounds.extent);
			}
			if (pendingMesh.shadowModel && pendingMesh.shadowModel != pendingMesh.model) {
				for (sp::Mesh &mesh : pendingMesh.shadowModel->meshes) {
					aabbMin = sf::min(aabbMin, mesh.bounds.origin - mesh.bounds.extent);
					aabbMax = sf::max(aabbMax, mesh.bounds.origin + mesh.bounds.extent);
				}
			}
			mesh.bounds = sf::Bounds3::minMax(aabbMin, aabbMax);
			mesh.localTransform = getModelComponentTransform(*pendingMesh.component);

			const sf::Mat34 &entityTransform = clientGlobalState->entities[entityId].transform.asMatrix();
			mesh.worldTransform = entityTransform * mesh.localTransform;

			sf::Bounds3 bounds = sf::transformBounds(mesh.worldTransform, mesh.bounds);
			areaState->addWorldBox(AreaMesh, meshId, entityId, bounds);
		}

		imp->pendingMeshes.removeSwap(i--);
	}
}

void MeshState::renderShadows(sf::Slice<const SpatialNode*> visibleNodes, const RenderArgs &args)
{
	MeshStateImp *imp = (MeshStateImp*)this;

	for (const SpatialNode *node : visibleNodes) {
		for (const BoxArea &box : node->boxes) {
			if (box.groupId != AreaMesh) continue;
			if (!args.frustum.intersects(box.bounds)) continue;

			uint32_t meshId = box.userId;
			MeshImp &mesh = imp->meshes[meshId];

		}
	}
}

void MeshState::renderMeshses(const RenderArgs &args)
{
	MeshStateImp *imp = (MeshStateImp*)this;
	AreaState *areaState = clientGlobalState->areaState;
	const AreaGroupState &groupState = areaState->getGroupState(AreaMesh);

	for (const AreaIds &ids : groupState.visible) {
		uint32_t meshId = ids.userId;
		MeshImp &meshImp = imp->meshes[meshId];

		for (sp::Mesh &mesh : meshImp.model->meshes) {

			gameShaders.debugMeshPipe.bind();

			sf::Mat44 meshToClip = args.worldToClip * meshImp.worldTransform;

			DebugMesh_Vertex_t ubo;
			meshToClip.writeColMajor44(ubo.worldToClip);
			sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_DebugMesh_Vertex, &ubo, sizeof(ubo));

			sg_bindings binds = { };
			binds.index_buffer = meshImp.model->indexBuffer.buffer;
			binds.index_buffer_offset = mesh.indexBufferOffset;
			binds.vertex_buffers[0] = meshImp.model->vertexBuffer.buffer;
			binds.vertex_buffer_offsets[0] = mesh.streams[0].offset;
			sg_apply_bindings(&binds);

			sg_draw(0, mesh.numIndices, 1);
		}
	}
}

float MeshState::castRay(uint32_t meshId, const sf::Ray &ray, float tMin) const
{
	const MeshStateImp *imp = (const MeshStateImp*)this;
	const MeshImp &meshImp = imp->meshes[meshId];
	float t = meshImp.model->castModelRay(ray, meshImp.worldTransform, tMin);
	return t;
}

}
