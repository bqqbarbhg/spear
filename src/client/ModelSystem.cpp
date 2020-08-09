#include "ModelSystem.h"

#include "client/AreaSystem.h"

#include "sf/Array.h"

#include "sp/Model.h"

// TEMP
#include "game/shader/GameShaders.h"
#include "game/shader/DebugMesh.h"

namespace cl {

struct ModelSystemImp final : ModelSystem
{
	struct Model
	{
		uint32_t areaId = ~0u;
		uint32_t entityId;

		uint32_t loadQueueIndex = ~0u;
		sp::ModelRef model;
		sp::ModelRef shadowModel;

		sf::Bounds3 modelBounds;

		sf::Mat34 modelToEntity;
		sf::Mat34 modelToWorld;

		bool isLoading() const {
			return model.isLoading() && (shadowModel && shadowModel.isLoading());
		}
		bool isLoaded() const {
			return model.isLoaded() && (!shadowModel || shadowModel.isLoaded());
		}
	};

	static sf::Mat34 getComponentTransform(const sv::ModelComponent &c)
	{
		return sf::mat::translate(c.position) * (
		sf::mat::rotateZ(c.rotation.z * (sf::F_PI/180.0f)) *
		sf::mat::rotateY(c.rotation.y * (sf::F_PI/180.0f)) *
		sf::mat::rotateX(c.rotation.x * (sf::F_PI/180.0f)) *
		sf::mat::scale(c.stretch * c.scale * 0.01f));
	}

	sf::Array<Model> models;
	sf::Array<uint32_t> freeModelIds;

	sf::Array<uint32_t> loadQueue;

	void finishLoadingModel(AreaSystem *areaSystem, uint32_t modelId)
	{
		Model &model = models[modelId];

		model.modelBounds = model.model->bounds;
		if (model.shadowModel) {
			model.modelBounds = sf::boundsUnion(model.modelBounds, model.shadowModel->bounds);
		}

		if (model.areaId != ~0u) {
			areaSystem->updateBoxArea(model.areaId, model.modelBounds, model.modelToWorld);
		} else {
			uint32_t areaFlags = Area::Visibilty | Area::EditorPick;
			if (model.shadowModel) areaFlags |= Area::Shadow;
			model.areaId = areaSystem->addBoxArea(AreaGroup::Model, modelId, model.modelBounds, model.modelToWorld, areaFlags);
		}
	}

	void startLoadingModel(AreaSystem *areaSystem, uint32_t modelId)
	{
		Model &model = models[modelId];
		if (model.isLoaded()) {
			finishLoadingModel(areaSystem, modelId);
		} else if (model.loadQueueIndex == ~0u && model.isLoading()) {
			model.loadQueueIndex = loadQueue.size;
			loadQueue.push(modelId);
		}
	}

	// API

	void addModel(Systems &systems, uint32_t entityId, uint8_t componentIndex, const sv::ModelComponent &c, const Transform &transform)
	{
		uint32_t modelId = models.size;
		if (freeModelIds.size > 0) {
			modelId = freeModelIds.popValue();
		} else {
			models.push();
		}

		Model &model = models[modelId];
		model.entityId = entityId;

		model.modelToEntity = getComponentTransform(c);
		model.modelToWorld = transform.asMatrix() * model.modelToEntity;

		model.model.load(c.model);
		if (c.castShadows) {
			model.shadowModel.load(c.shadowModel ? c.shadowModel : c.model);
		}
		startLoadingModel(systems.area, modelId);

		systems.entities.addComponent(entityId, this, modelId, 0, componentIndex, Entity::UpdateTransform);
	}

	void updateTransform(Systems &systems, const EntityComponent &ec, const TransformUpdate &update)
	{
		uint32_t modelId = ec.userId;
		Model &model = models[modelId];

		model.modelToWorld = update.entityToWorld * model.modelToEntity;

		if (model.areaId != ~0u) {
			systems.area->updateBoxArea(model.areaId, model.modelBounds, model.modelToWorld);
		}
	}

	void remove(Systems &systems, const EntityComponent &ec)
	{
		uint32_t modelId = ec.userId;
		Model &model = models[modelId];

		if (model.areaId != ~0u) {
			systems.area->removeBoxArea(model.areaId);
		}

		freeModelIds.push(modelId);
		sf::reset(model);
	}

	void updateLoadQueue(AreaSystem *areaSystem)
	{
		for (uint32_t i = 0; i < loadQueue.size; i++) {
			uint32_t modelId = loadQueue[i];
			Model &model = models[modelId];
			if (model.isLoading()) continue;

			if (model.isLoaded()) {
				finishLoadingModel(areaSystem, modelId);
			}

			models[loadQueue.back()].loadQueueIndex = i;
			model.loadQueueIndex = ~0u;
			loadQueue.removeSwap(i--);
		}
	}

	void renderMain(const VisibleAreas &visibleAreas, const RenderArgs &renderArgs)
	{
		for (uint32_t modelId : visibleAreas.get(AreaGroup::Model)) {
			Model &model = models[modelId];

			for (sp::Mesh &mesh : model.model->meshes) {

				gameShaders.debugMeshPipe.bind();

				sf::Mat44 meshToClip = renderArgs.worldToClip * model.modelToWorld;

				DebugMesh_Vertex_t ubo;
				meshToClip.writeColMajor44(ubo.worldToClip);
				sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_DebugMesh_Vertex, &ubo, sizeof(ubo));

				sg_bindings binds = { };
				binds.index_buffer = model.model->indexBuffer.buffer;
				binds.index_buffer_offset = mesh.indexBufferOffset;
				binds.vertex_buffers[0] = model.model->vertexBuffer.buffer;
				binds.vertex_buffer_offsets[0] = mesh.streams[0].offset;
				sg_apply_bindings(&binds);

				sg_draw(0, mesh.numIndices, 1);
			}

		}
	}
};

sf::Box<ModelSystem> ModelSystem::create() { return sf::box<ModelSystemImp>(); }

}
