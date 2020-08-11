#include "CharacterModelSystem.h"

#include "client/AreaSystem.h"

#include "sf/Array.h"

#include "sp/Model.h"
#include "sp/Animation.h"

#include "game/DebugDraw.h"

// TEMP
#include "game/shader/GameShaders.h"
#include "game/shader/DebugSkinnedMesh.h"

namespace cl {

struct CharacterModelSystemImp final : CharacterModelSystem
{
	struct Animation
	{
		sf::Symbol name;
		sp::AnimationRef animation;
		sf::Array<uint32_t> boneMapping;
	};

	struct Model
	{
		uint32_t areaId = ~0u;
		uint32_t entityId;

		uint32_t loadQueueIndex = ~0u;
		sp::ModelRef model;

		sf::ImplicitHashMap<Animation, sv::KeyName> animations;

		sf::Mat34 modelToEntity;
		sf::Mat34 modelToWorld;

		float hackAnimTime = 0.0f;

		bool isLoading() const {
			for (const Animation &anim : animations) {
				if (anim.animation.isLoading()) return true;
			}
			return model.isLoading();
		}

		bool isLoaded() const {
			for (const Animation &anim : animations) {
				if (!anim.animation.isLoaded()) return false;
			}
			return model.isLoaded();
		}
	};

	sf::Array<Model> models;
	sf::Array<uint32_t> freeModelIds;

	sf::Array<uint32_t> loadQueue;

	void finishLoadingModel(AreaSystem *areaSystem, uint32_t modelId)
	{
		Model &model = models[modelId];

		sf::Bounds3 bounds;
		bounds.origin = model.modelToWorld.cols[3];
		bounds.extent = sf::Vec3(3.0f); // TEMP HACK TODO

		for (Animation &anim : model.animations) {
			anim.boneMapping.resizeUninit(anim.animation->bones.size);
			anim.animation->generateBoneMapping(model.model, anim.boneMapping);
		}

		if (model.areaId != ~0u) {
			areaSystem->updateBoxArea(model.areaId, bounds);
		} else {
			uint32_t areaFlags = Area::Visibilty;
			model.areaId = areaSystem->addBoxArea(AreaGroup::CharacterModel, modelId, bounds, areaFlags);
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

	void addCharacterModel(Systems &systems, uint32_t entityId, uint8_t componentIndex, const sv::CharacterModelComponent &c, const Transform &transform) override
	{
		uint32_t modelId = models.size;
		if (freeModelIds.size > 0) {
			modelId = freeModelIds.popValue();
		} else {
			models.push();
		}

		Model &model = models[modelId];
		model.entityId = entityId;

		model.modelToEntity = sf::mat::scale(0.01f);
		model.modelToWorld = transform.asMatrix() * model.modelToEntity;

		model.model.load(c.modelName);

		model.animations.reserve(c.animations.size);
		for (const sv::AnimationInfo &info : c.animations) {
			Animation anim;
			anim.name = info.name;
			anim.animation.load(info.file);
			model.animations.insert(std::move(anim));
		}

		startLoadingModel(systems.area, modelId);

		systems.entities.addComponent(entityId, this, modelId, 0, componentIndex, Entity::UpdateTransform);
	}

	void updateTransform(Systems &systems, const EntityComponent &ec, const TransformUpdate &update) override
	{
		uint32_t modelId = ec.userId;
		Model &model = models[modelId];

		model.modelToWorld = update.entityToWorld * model.modelToEntity;

		sf::Bounds3 bounds;
		bounds.origin = model.modelToWorld.cols[3];
		bounds.extent = sf::Vec3(3.0f); // TEMP HACK TODO

		if (model.areaId != ~0u) {
			systems.area->updateBoxArea(model.areaId, bounds);
		}
	}

	void remove(Systems &systems, const EntityComponent &ec) override
	{
		uint32_t modelId = ec.userId;
		Model &model = models[modelId];

		if (model.areaId != ~0u) {
			systems.area->removeBoxArea(model.areaId);
		}

		freeModelIds.push(modelId);
		sf::reset(model);
	}

	void updateLoadQueue(AreaSystem *areaSystem) override
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

	void renderMain(const VisibleAreas &visibleAreas, const RenderArgs &renderArgs) override
	{
		sf::SmallArray<sp::BoneTransform, sp::MaxBones> boneTransforms;
		sf::SmallArray<sf::Mat34, sp::MaxBones> boneWorld;
		for (uint32_t modelId : visibleAreas.get(AreaGroup::CharacterModel)) {
			Model &model = models[modelId];

			boneTransforms.resizeUninit(model.model->bones.size);
			boneWorld.resizeUninit(model.model->bones.size);


			{
				Animation &anim = model.animations.data[0];
				model.hackAnimTime += 1.0f/60.0f;
				if (model.hackAnimTime > anim.animation->duration) {
					model.hackAnimTime -= anim.animation->duration;
				}
				anim.animation->evaluate(model.hackAnimTime, anim.boneMapping, boneTransforms);
			}

			sp::boneTransformToWorld(model.model, boneWorld, boneTransforms, model.modelToWorld);

			for (sp::Mesh &mesh : model.model->meshes) {

				gameShaders.debugSkinnedMeshPipe.bind();

				DebugSkinnedMesh_SkinTransform_t vu;
				renderArgs.worldToClip.writeColMajor44(vu.worldToClip);

				DebugSkinnedMesh_Bones_t bones;

				for (uint32_t i = 0; i < mesh.bones.size; i++) {
					sp::MeshBone &meshBone = mesh.bones[i];
					sf::Mat34 transform = boneWorld[meshBone.boneIndex] * meshBone.meshToBone;
					memcpy(bones.bones[i * 3 + 0].v, transform.getRow(0).v, sizeof(sf::Vec4));
					memcpy(bones.bones[i * 3 + 1].v, transform.getRow(1).v, sizeof(sf::Vec4));
					memcpy(bones.bones[i * 3 + 2].v, transform.getRow(2).v, sizeof(sf::Vec4));
				}

				sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_DebugSkinnedMesh_Bones, &bones, sizeof(bones));
				sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_DebugSkinnedMesh_SkinTransform, &vu, sizeof(vu));

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

sf::Box<CharacterModelSystem> CharacterModelSystem::create() { return sf::box<CharacterModelSystemImp>(); }

}
