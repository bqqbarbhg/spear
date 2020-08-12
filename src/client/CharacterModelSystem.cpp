#include "CharacterModelSystem.h"

#include "client/AreaSystem.h"

#include "sf/Array.h"

#include "sp/Model.h"
#include "sp/Animation.h"

#include "sf/Random.h"

#include "game/DebugDraw.h"

// TEMP
#include "game/shader/GameShaders.h"
#include "game/shader/DebugSkinnedMesh.h"
#include "ext/imgui/imgui.h"

namespace cl {

struct CharacterModelSystemImp final : CharacterModelSystem
{
	struct Animation
	{
		sp::AnimationRef animation;
		sf::Array<uint32_t> boneMapping;
		sf::Array<sf::Symbol> tags;
		float weight;
		float speed;
		float speedVariation;
		float fadeOutTime = 0.4f;
		bool loop = true;
	};

	struct AttachBone
	{
		sf::Symbol boneName;
		uint32_t boneIndex;
		float scale;
	};

	struct ActiveAnimation
	{
		uint32_t animIndex;
		float alphaVelocity = 2.0f;
		float alpha = 0.0f;
		float time = 0.0f;
		float speed = 1.0f;
		bool loop = true;
	};

	struct Model
	{
		uint32_t areaId = ~0u;
		uint32_t entityId;

		uint32_t loadQueueIndex = ~0u;
		sp::ModelRef model;

		sf::Array<Animation> animations;
		sf::SmallArray<uint32_t, 2> attachIds;

		sf::Array<sf::Mat34> boneToWorld;
		sf::HashMap<sf::Symbol, AttachBone> attachBones;

		sf::Mat34 modelToEntity;
		sf::Mat34 modelToWorld;

		sf::Array<ActiveAnimation> activeAnimations;

		sf::HashMap<sf::Symbol, int32_t> tags;

		bool isLoading() const {
			for (const Animation &anim : animations) {
				if (anim.animation.isLoading()) return true;
			}
			return model.isLoading();
		}

		bool isLoaded() const {
			for (const Animation &anim : animations) {
				if (!anim.animation.isLoaded()) {
					return false;
				}
			}
			return model.isLoaded();
		}

		void addTag(const sf::Symbol &tag)
		{
			tags[tag]++;
		}

		void removeTag(const sf::Symbol &tag)
		{
			if (--tags[tag] < 0) {
				tags.remove(tag);
			}
		}
	};

	struct Attachment
	{
		uint32_t modelId = ~0u;
		uint32_t entityId = ~0u;

		sf::Mat34 childToBone;
		float scale = 1.0f;

		sf::Symbol boneName;
		uint32_t boneIndex = ~0u;

		sf::Array<sf::Symbol> animationTags;
	};

	struct AnimWorkCtx
	{
		sf::Random rng;
		sf::Array<sp::BoneTransform> boneTransforms;
		sf::Array<sp::BoneTransform> tempBoneTransforms;
	};

	sf::Array<Model> models;
	sf::Array<uint32_t> freeModelIds;

	sf::Array<Attachment> attachments;
	sf::Array<uint32_t> freeAttachmentIds;

	sf::Array<uint32_t> attachmentsToUpdate;

	sf::Array<uint32_t> loadQueue;

	sf::HashMap<sf::Symbol, double> tagWeights;

	AnimWorkCtx animCtx;

	CharacterModelSystemImp()
	{
		tagWeights[sf::Symbol("Idle")] = 10.0;
	}

	double getTagWeight(const sf::Symbol &tag)
	{
		if (const double *value = tagWeights.findValue(tag)) {
			return *value;
		} else {
			return 1.0;
		}
	}

	void finishLoadingAttachmentImp(Attachment &attach, Model &model)
	{
		if (AttachBone *attachBone = model.attachBones.findValue(attach.boneName)) {
			attach.boneIndex = attachBone->boneIndex;
			attach.scale = attachBone->scale;
		}
	}

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

		for (auto &pair : model.attachBones) {
			if (const uint32_t *boneIx = model.model->boneNames.findValue(pair.val.boneName)) {
				pair.val.boneIndex = *boneIx;
			}
		}

		for (uint32_t attachId : model.attachIds) {
			Attachment &attach = attachments[attachId];
			finishLoadingAttachmentImp(attach, model);
		}

		model.boneToWorld.resize(model.model->bones.size);

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

	void updateAnimationStateImp(AnimWorkCtx &ctx, Model &model)
	{
		if (model.animations.size == 0) return;

		double bestScore = 0.0;
		uint32_t bestAnim = 0;

		for (const Animation &anim : model.animations) {
			double score = 0.0;
			for (const sf::Symbol &tag : anim.tags) {
				if (int32_t *count = model.tags.findValue(tag)) {
					score += (double)*count * getTagWeight(tag);
				} else {
					score -= 10.0f * getTagWeight(tag);
				}
			}

			score += (double)anim.weight * (double)ctx.rng.nextFloat() * 0.01;

			if (score > bestScore) {
				bestScore = score;
				bestAnim = (uint32_t)(&anim - model.animations.data);
			}
		}

		Animation &nextAnim = model.animations[bestAnim];
		float nextDuration = nextAnim.animation->duration;
		float speed = nextAnim.speed + ctx.rng.nextFloat() * nextAnim.speedVariation;

		if (model.activeAnimations.size == 0) {
			ActiveAnimation &next = model.activeAnimations.push();
			next.animIndex = bestAnim;
			next.alphaVelocity = 0.0f;
			next.alpha = 1.0f;
			next.speed = speed;
		} else {
			ActiveAnimation &top = model.activeAnimations.back();
			if (top.animIndex == bestAnim && top.speed == speed && nextAnim.loop) {
				top.time -= nextDuration;
			} else {
				Animation &topAnim = model.animations[top.animIndex];
				float fadeOut = (topAnim.animation->duration - top.time) / speed;

				ActiveAnimation &next = model.activeAnimations.push();
				// top invalidated!

				next.animIndex = bestAnim;
				next.alphaVelocity = 1.0f / fadeOut * 1.05f;
				next.alpha = 0.0f;
				next.speed = speed;

				if (nextAnim.loop) {
					next.time = -fadeOut;
				}
			}
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

		model.modelToEntity = sf::mat::scale(0.01f * c.scale);
		model.modelToWorld = transform.asMatrix() * model.modelToEntity;

		model.model.load(c.modelName);

		model.animations.reserve(c.animations.size);
		for (const sv::AnimationInfo &info : c.animations) {
			Animation &anim = model.animations.push();
			anim.animation.load(info.file);
			anim.tags = info.tags;
			anim.weight = info.weight;
			anim.speed = info.speed;
			anim.speedVariation = info.speedVariation;
			anim.loop = info.loop;
		}

		model.attachBones.reserve(c.attachBones.size);
		for (const sv::AttachBone &desc : c.attachBones) {
			AttachBone &attachBone = model.attachBones[desc.name];
			attachBone.boneName = desc.boneName;
			attachBone.boneIndex = ~0u;
			attachBone.scale = desc.scale;
		}

		model.addTag(sf::Symbol("Always"));

		// TEMP HACK TEMP HACK
		model.addTag(sf::Symbol("Idle"));

		startLoadingModel(systems.area, modelId);

		systems.entities.addComponent(entityId, this, modelId, 0, componentIndex, Entity::UpdateTransform);
	}

	void addAttachedEntity(Systems &systems, uint32_t parentEntityId, uint32_t childEntityId, const AttachDesc &desc) override
	{
		Entity &parentEntity = systems.entities.entities[parentEntityId];

		for (cl::EntityComponent &ec : parentEntity.components) {
			if (ec.system == this && ec.subsystemIndex == 0) {
				uint32_t modelId = ec.userId;
				Model &model = models[modelId];

				uint32_t attachId = attachments.size;
				if (freeAttachmentIds.size > 0) {
					attachId = freeAttachmentIds.popValue();
				} else {
					attachments.push();
				}

				Attachment &attach = attachments[attachId];
				attach.modelId = modelId;
				attach.entityId = childEntityId;
				attach.boneName = desc.boneName;
				attach.childToBone = desc.childToBone;

				model.attachIds.push(attachId);

				attach.animationTags.push(desc.animationTags);
				for (const sf::Symbol &tag : desc.animationTags) {
					model.addTag(tag);
				}

				if (model.loadQueueIndex == ~0u && model.model.isLoaded()) {
					finishLoadingAttachmentImp(attach, model);
				}

				systems.entities.addComponent(childEntityId, this, attachId, 1, 0xff, 0);

				return;
			}
		}
	}

	void updateTransform(Systems &systems, uint32_t entityId, const EntityComponent &ec, const TransformUpdate &update) override
	{
		sf_assert(ec.subsystemIndex == 0);

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

	void remove(Systems &systems, uint32_t entityId, const EntityComponent &ec) override
	{
		if (ec.subsystemIndex == 0) {
			uint32_t modelId = ec.userId;
			Model &model = models[modelId];

			if (model.areaId != ~0u) {
				systems.area->removeBoxArea(model.areaId);
			}

			for (uint32_t attachId : model.attachIds) {
				Attachment &attach = attachments[attachId];
				attach.modelId = ~0u;
			}

			freeModelIds.push(modelId);
			sf::reset(model);
		} else if (ec.subsystemIndex == 1) {
			uint32_t attachId = ec.userId;
			Attachment &attach = attachments[attachId];

			if (attach.modelId != ~0u) {
				Model &model = models[attach.modelId];

				for (const sf::Symbol &tag : attach.animationTags) {
					model.removeTag(tag);
				}

				sf::findRemoveSwap(model.attachIds, attachId);
			}

			freeAttachmentIds.push(attachId);
			sf::reset(attach);
		}
	}

	void updateAnimations(const VisibleAreas &visibleAreas, float dt) override
	{
		AnimWorkCtx &ctx = animCtx;

		for (uint32_t modelId : visibleAreas.get(AreaGroup::CharacterModel)) {
			Model &model = models[modelId];

			ctx.boneTransforms.clear();
			ctx.boneTransforms.resize(model.model->bones.size);

			bool needStateUpdate = false;

			if (model.activeAnimations.size == 0) {
				needStateUpdate = true;
			} else {
				ActiveAnimation &top = model.activeAnimations.back();
				Animation &topAnim = model.animations[top.animIndex];
				float threshold = topAnim.animation->duration - topAnim.fadeOutTime;
				if (top.time < threshold && top.time + dt * top.speed >= threshold) {
					needStateUpdate = true;
				}
			}

			if (needStateUpdate) {
				updateAnimationStateImp(ctx, model);
			}

			for (uint32_t i = 0; i < model.activeAnimations.size; i++) {
				ActiveAnimation &active = model.activeAnimations[i];
				Animation &anim = model.animations[active.animIndex];
				active.time += dt * active.speed;

				ImGui::Text("%+.2f/%.2f (%.2fx)  %s", active.time, anim.animation->duration, active.speed, anim.animation->name.data);

				float duration = anim.animation->duration;
				float t = active.time;
				if (t < 0.0f) {
					t += duration;
					if (t < 0.0f) {
						t = duration - fmodf(-t, duration);
					}
				} else if (t > duration) {
					t -= duration;
					if (t > duration) {
						t = fmodf(t, duration);
					}
				}

				float a = active.alpha;
				a = a * a * (3.0f - 2.0f * a);

				const float loopFadeDuration = 1.0f/30.0f;
				float left = duration - t;
				if (anim.loop && left < loopFadeDuration) {
					ctx.tempBoneTransforms.clear();
					ctx.tempBoneTransforms.resize(model.model->bones.size);

					float endAlpha = 1.0f - left / loopFadeDuration;
					anim.animation->evaluate(t, anim.boneMapping, ctx.tempBoneTransforms);
					anim.animation->evaluate(0.0f, anim.boneMapping, ctx.tempBoneTransforms, endAlpha);

					sp::blendBoneTransform(ctx.boneTransforms, ctx.tempBoneTransforms, a);

				} else if (i == 0 || active.alpha >= 0.9999f) {
					anim.animation->evaluate(t, anim.boneMapping, ctx.boneTransforms);
				} else {
					anim.animation->evaluate(t, anim.boneMapping, ctx.boneTransforms, a);
				}

				if (active.alpha < 1.0f) {
					active.alpha += dt * active.alphaVelocity;
				} else if (i > 0) {
					active.alpha = 1.0f;
					model.activeAnimations.removeOrdered(0, i);
					i = 0;
				}
			}

			sp::boneTransformToWorld(model.model, model.boneToWorld, ctx.boneTransforms, model.modelToWorld);

			// TODO(threads): Mutex
			attachmentsToUpdate.push(model.attachIds);
		}
	}

	void updateAttachedEntities(Systems &systems) override
	{
		for (uint32_t attachId : attachmentsToUpdate) {
			Attachment &attach = attachments[attachId];
			if (attach.boneIndex == ~0u || attach.modelId == ~0u) continue;
			Model &model = models[attach.modelId];

			sf::Mat34 childToWorld = model.boneToWorld[attach.boneIndex] * sf::mat::scale(100.0f) * attach.childToBone;
			float scale = cbrtf(sf::abs(sf::determinant(childToWorld)));

			sf::Vec3 z = sf::normalize(childToWorld.cols[2]);
			sf::Vec3 y = sf::normalize(childToWorld.cols[1]);
			sf::Vec3 x = sf::normalize(sf::cross(childToWorld.cols[1], childToWorld.cols[2]));
			y = sf::normalize(sf::cross(z, x));

			Transform transform;
			transform.position = childToWorld.cols[3];
			transform.rotation = sf::axesToQuat(x, y, z);
			transform.scale = scale * attach.scale;
			systems.entities.updateTransform(systems, attach.entityId, transform);
		}

		attachmentsToUpdate.clear();
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
		for (uint32_t modelId : visibleAreas.get(AreaGroup::CharacterModel)) {
			Model &model = models[modelId];

			for (sp::Mesh &mesh : model.model->meshes) {

				gameShaders.debugSkinnedMeshPipe.bind();

				DebugSkinnedMesh_SkinTransform_t vu;
				renderArgs.worldToClip.writeColMajor44(vu.worldToClip);

				DebugSkinnedMesh_Bones_t bones;

				for (uint32_t i = 0; i < mesh.bones.size; i++) {
					sp::MeshBone &meshBone = mesh.bones[i];
					sf::Mat34 transform = model.boneToWorld[meshBone.boneIndex] * meshBone.meshToBone;
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
