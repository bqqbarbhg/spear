#include "CharacterModelSystem.h"

#include "client/AreaSystem.h"
#include "client/LightSystem.h"

#include "game/shader2/GameShaders2.h"
#include "client/MeshMaterial.h"

#include "sf/Array.h"

#include "sp/Model.h"
#include "sp/Animation.h"

#include "sf/Random.h"

#include "game/DebugDraw.h"

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

	struct Material
	{
		sf::Symbol name;
		MeshMaterialRef material;
	};

	struct Model
	{
		uint32_t areaId = ~0u;
		uint32_t entityId;

		uint32_t loadQueueIndex = ~0u;
		sp::ModelRef model;
		sf::SmallArray<Material, 4> materials;

		sf::Array<Animation> animations;
		sf::SmallArray<uint32_t, 2> attachIds;

		sf::Array<sf::Mat34> boneToWorld;
		sf::HashMap<sf::Symbol, AttachBone> attachBones;

		sf::Mat34 modelToEntity;
		sf::Mat34 modelToWorld;

		sf::Bounds3 bounds;

		sf::Array<ActiveAnimation> activeAnimations;

		sf::HashMap<sf::Symbol, int32_t> tags;

		double lastUpdateTime = 0.0;

		bool isLoading() const {
			for (const Animation &anim : animations) {
				if (anim.animation.isLoading()) return true;
			}
			for (const Material &mat : materials) {
				if (mat.material.isLoading()) return true;
			}
			return model.isLoading();
		}

		bool isLoaded() const {
			for (const Animation &anim : animations) {
				if (!anim.animation.isLoaded()) return false;
			}
			for (const Material &mat : materials) {
				if (!mat.material.isLoaded()) return false;
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
		double updateTime = 0.0;
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

	Shader2 skinShader;
	sp::Pipeline skinPipe;

	AnimWorkCtx animCtx;

	double getTagWeight(const sf::Symbol &tag)
	{
		if (const double *value = tagWeights.findValue(tag)) {
			return *value;
		} else {
			return 1.0;
		}
	}

	void updateAttachmentTransformImp(Systems &systems, Attachment &attach, Model &model)
	{
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

		for (Animation &anim : model.animations) {
			anim.boneMapping.resizeUninit(anim.animation->bones.size);
			anim.animation->generateBoneMapping(model.model, anim.boneMapping);

			float maxFadeOut = anim.animation->duration * 0.75f;
			anim.fadeOutTime = sf::min(anim.fadeOutTime, maxFadeOut);
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
			areaSystem->updateBoxArea(model.areaId, model.bounds);
		} else {
			uint32_t areaFlags = Area::Activate | Area::Visibility;
			model.areaId = areaSystem->addBoxArea(AreaGroup::CharacterModel, modelId, model.bounds, areaFlags);
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

	static sf_forceinline float wrapTime(float t, float duration)
	{
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
		return t;
	}

	void updateAnimationStateImp(AnimWorkCtx &ctx, Model &model, float dt)
	{
		if (model.animations.size == 0) return;

		double bestScore = -HUGE_VAL;
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

		bool simulateCatchUp = false;

		if (model.activeAnimations.size > 0) {
			ActiveAnimation &top = model.activeAnimations.back();
			if (top.animIndex == bestAnim && top.speed == speed && nextAnim.loop) {
				top.time = wrapTime(top.time, nextDuration) - nextDuration;

				// Early return: Skip fallback
				return;

			} else {
				Animation &topAnim = model.animations[top.animIndex];
				float timeLeft = topAnim.animation->duration - top.time;

				if (timeLeft > 0.0f) {
					float fadeOut = timeLeft / speed;
					ActiveAnimation &next = model.activeAnimations.push();
					// top invalidated!

					next.animIndex = bestAnim;
					next.alphaVelocity = 1.0f / fadeOut * 1.05f;
					next.alpha = 0.0f;
					next.speed = speed;

					if (nextAnim.loop) {
						next.time = -fadeOut;
					}

					// Early return: Skip fallback
					return;
				} else {
					if (timeLeft < -0.1f) {
						simulateCatchUp = true;
					}
				}
			}
		}

		model.activeAnimations.clear();

		// Fallback: Set the next animation directly
		ActiveAnimation &next = model.activeAnimations.push();
		next.animIndex = bestAnim;
		next.alphaVelocity = 0.0f;
		next.alpha = 1.0f;
		next.speed = speed;
		next.time = 0.0f;

		if (nextAnim.loop || simulateCatchUp) {
			float timeRange = nextDuration - nextAnim.fadeOutTime * 1.25f;
			if (timeRange > 0.0f) {
				next.time = ctx.rng.nextFloat() * timeRange;
			}
		}
	}

	// API

	CharacterModelSystemImp(const SystemsDesc &desc)
	{
		tagWeights[sf::Symbol("Idle")] = 10.0;
		animCtx.rng = sf::Random(desc.seed[0], 756789);

		uint8_t permutation[SP_NUM_PERMUTATIONS] = { };
		#if CL_SHADOWCACHE_USE_ARRAY
			permutation[SP_SHADOWGRID_USE_ARRAY] = 1;
		#else
			permutation[SP_SHADOWGRID_USE_ARRAY] = 0;
		#endif
		permutation[SP_NORMALMAP_REMAP] = MeshMaterial::useNormalMapRemap;
		skinShader = getShader2(SpShader_TestSkin, permutation);

		{
			uint32_t flags = sp::PipeDepthWrite|sp::PipeCullCCW;
			flags |= sp::PipeIndex16;
			auto &d = skinPipe.init(skinShader.handle, flags);
			d.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
			d.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT2;
			d.layout.attrs[2].format = SG_VERTEXFORMAT_SHORT4N;
			d.layout.attrs[3].format = SG_VERTEXFORMAT_SHORT4N;
			d.layout.attrs[4].format = SG_VERTEXFORMAT_UBYTE4;
			d.layout.attrs[5].format = SG_VERTEXFORMAT_UBYTE4N;
		}
	}

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
		for (auto &material : c.materials) {
			if (!material.material) continue;
			Material &mat = model.materials.push();
			mat.name = material.name;
			mat.material.load(material.material);
		}

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
		model.bounds.origin = transform.position;
		model.bounds.extent = sf::Vec3(2.0f);

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
					updateAttachmentTransformImp(systems, attach, model);
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

		model.bounds.origin = update.transform.position;
		model.bounds.extent = sf::Vec3(2.0f);

		if (model.areaId != ~0u) {
			systems.area->updateBoxArea(model.areaId, model.bounds);
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

	void updateAnimations(const VisibleAreas &activeAreas, float realDt) override
	{
		AnimWorkCtx &ctx = animCtx;

		ctx.updateTime += realDt;

		for (uint32_t modelId : activeAreas.get(AreaGroup::CharacterModel)) {
			Model &model = models[modelId];

			float dt = (float)(ctx.updateTime - model.lastUpdateTime);
			model.lastUpdateTime = ctx.updateTime;

			ctx.boneTransforms.clear();
			ctx.boneTransforms.resize(model.model->bones.size);

			bool needStateUpdate = false;

			if (model.activeAnimations.size == 0) {
				needStateUpdate = true;
			} else {
				ActiveAnimation &top = model.activeAnimations.back();
				Animation &topAnim = model.animations[top.animIndex];
				float threshold = topAnim.animation->duration - topAnim.fadeOutTime;
				if (top.time >= threshold) {
					needStateUpdate = true;
				}
			}

			if (needStateUpdate) {
				updateAnimationStateImp(ctx, model, dt);
			}

			for (uint32_t i = 0; i < model.activeAnimations.size; i++) {
				ActiveAnimation &active = model.activeAnimations[i];
				Animation &anim = model.animations[active.animIndex];

				// ImGui::Text("%+.2f/%.2f (%.2fx %.2f alpha)  %s", active.time, anim.animation->duration, active.speed, active.alpha, anim.animation->name.data);

				float duration = anim.animation->duration;
				float t = wrapTime(active.time, duration);

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

				active.time += dt * active.speed;
				if (active.alpha < 1.0f) {
					active.alpha += dt * active.alphaVelocity;
					if (active.alpha >= 1.0f) {
						active.alpha = 1.0f;
						if (i > 0) {
							model.activeAnimations.removeOrdered(0, i);
							i = 0;
						}
					}
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
			updateAttachmentTransformImp(systems, attach, model);
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

	void renderMain(LightSystem *lightSystem, const VisibleAreas &visibleAreas, const RenderArgs &renderArgs) override
	{
		sf::SmallArray<cl::PointLight, 64> pointLights;
		const uint32_t maxLights = 16;

		UBO_SkinTransform tu;
		UBO_Pixel pu;
		UBO_Bones bones;

		sg_bindings bindings = { };
		bindImageFS(skinShader, bindings, CL_SHADOWCACHE_TEX, lightSystem->getShadowTexture());

		tu.worldToClip = renderArgs.worldToClip;

		for (uint32_t modelId : visibleAreas.get(AreaGroup::CharacterModel)) {
			Model &model = models[modelId];

			for (sp::Mesh &mesh : model.model->meshes) {

				if (skinPipe.bind()) {
					bindUniformVS(skinShader, tu);
				}

				for (uint32_t i = 0; i < mesh.bones.size; i++) {
					sp::MeshBone &meshBone = mesh.bones[i];
					sf::Mat34 transform = model.boneToWorld[meshBone.boneIndex] * meshBone.meshToBone;
					memcpy(bones.bones[i * 3 + 0].v, transform.getRow(0).v, sizeof(sf::Vec4));
					memcpy(bones.bones[i * 3 + 1].v, transform.getRow(1).v, sizeof(sf::Vec4));
					memcpy(bones.bones[i * 3 + 2].v, transform.getRow(2).v, sizeof(sf::Vec4));
				}

				bindUniformVS(skinShader, bones);

				pointLights.clear();
				lightSystem->queryVisiblePointLights(visibleAreas, pointLights, model.bounds);

				// TODO: Prioritize
				if (pointLights.size > maxLights) {
					pointLights.resizeUninit(maxLights);
				}

				pu.numLightsF = (float)pointLights.size;
				pu.cameraPosition = renderArgs.cameraPosition;
				sf::Vec4 *dst = pu.pointLightData;
				for (PointLight &light : pointLights) {
					light.writeShader(dst);
				}

				bindUniformFS(skinShader, pu);

				bool foundMaterial = false;
				for (const Material &mat : model.materials) {
					if (mat.name == mesh.materialName) {
						bindImageFS(skinShader, bindings, TEX_albedoTexture, mat.material->images[(uint32_t)MaterialTexture::Albedo]);
						bindImageFS(skinShader, bindings, TEX_normalTexture, mat.material->images[(uint32_t)MaterialTexture::Normal]);
						bindImageFS(skinShader, bindings, TEX_maskTexture, mat.material->images[(uint32_t)MaterialTexture::Mask]);
						foundMaterial = true;
						break;
					}
				}

				if (!foundMaterial) {
					bindImageFS(skinShader, bindings, TEX_albedoTexture, MeshMaterial::defaultImages[(uint32_t)MaterialTexture::Albedo]);
					bindImageFS(skinShader, bindings, TEX_normalTexture, MeshMaterial::defaultImages[(uint32_t)MaterialTexture::Normal]);
					bindImageFS(skinShader, bindings, TEX_maskTexture, MeshMaterial::defaultImages[(uint32_t)MaterialTexture::Mask]);
				}

				bindings.index_buffer = model.model->indexBuffer.buffer;
				bindings.index_buffer_offset = mesh.indexBufferOffset;
				bindings.vertex_buffers[0] = model.model->vertexBuffer.buffer;
				bindings.vertex_buffer_offsets[0] = mesh.streams[0].offset;
				sg_apply_bindings(&bindings);

				sg_draw(0, mesh.numIndices, 1);
			}

		}
	}

};

sf::Box<CharacterModelSystem> CharacterModelSystem::create(const SystemsDesc &desc) { return sf::box<CharacterModelSystemImp>(desc); }

}
