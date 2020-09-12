#include "ModelSystem.h"

#include "client/AreaSystem.h"
#include "client/LightSystem.h"
#include "client/EnvLightSystem.h"

#include "game/shader2/GameShaders2.h"
#include "client/MeshMaterial.h"

#include "sf/Array.h"

#include "sp/Model.h"

#include "game/DebugDraw.h"

namespace cl {

static const spmdl_attrib tileAttribs[] = {
	SP_VERTEX_ATTRIB_POSITION, SP_FORMAT_RGB32_FLOAT, 0, 0,
	SP_VERTEX_ATTRIB_NORMAL, SP_FORMAT_RGB32_FLOAT, 0, 3*4,
	SP_VERTEX_ATTRIB_TANGENT, SP_FORMAT_RGBA32_FLOAT, 0, 6*4,
	SP_VERTEX_ATTRIB_UV, SP_FORMAT_RG32_FLOAT, 0, 10*4,
};

static const spmdl_attrib packedAttribs[] = {
	SP_VERTEX_ATTRIB_POSITION, SP_FORMAT_RGB32_FLOAT, 0, 0,
	SP_VERTEX_ATTRIB_NORMAL, SP_FORMAT_RGBA16_SNORM, 0, 3*4,
	SP_VERTEX_ATTRIB_TANGENT, SP_FORMAT_RGBA16_SNORM, 0, 5*4,
	SP_VERTEX_ATTRIB_UV, SP_FORMAT_RG32_FLOAT, 0, 7*4,
};

static const sf::Slice<const spmdl_attrib> vertexFormatAttribs[] = {
	tileAttribs, packedAttribs
};

struct ModelSystemImp final : ModelSystem
{
	enum class VertexFormat
	{
		Tile,
		Packed,
		Count,
	};

	struct ModelData
	{
		sp::ModelRef model;
		sp::ModelRef shadowModel;
		MeshMaterialRef material;
	};

	struct Model
	{
		uint32_t areaId = ~0u;
		uint32_t entityId;

		VertexFormat vertexFormat;

		uint32_t loadQueueIndex = ~0u;
		sp::ModelRef model;
		sp::ModelRef shadowModel;
		cl::MeshMaterialRef material;

		sf::Bounds3 modelBounds;
		sf::Bounds3 worldBounds;

		sf::Mat34 modelToEntity;
		sf::Mat34 modelToWorld;

		bool isLoading() const {
			if (model.isLoading()) return true;
			if (shadowModel && shadowModel.isLoading()) return true;
			if (material && material.isLoading()) return true;
			return false;
		}
		bool isLoaded() const {
			if (!model.isLoaded()) return false;
			if (shadowModel && !shadowModel.isLoaded()) return false;
			if (material && !material.isLoaded()) return false;
			return true;
		}
	};

	static sf::Mat34 getComponentTransform(const sv::DynamicModelComponent &c)
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

	Shader2 meshShader;
	sp::Pipeline meshPipes[(uint32_t)VertexFormat::Count];

	void finishLoadingModel(AreaSystem *areaSystem, uint32_t modelId)
	{
		Model &model = models[modelId];

		VertexFormat vertexFormat = VertexFormat::Count;

		if (model.model->meshes.size > 0) {
			sp::Mesh &mesh = model.model->meshes[0];

			// Assume all meshes have the same vertex format
			for (uint32_t i = 0; i < (uint32_t)VertexFormat::Count; i++) {
				sf::Slice<const spmdl_attrib> ref = vertexFormatAttribs[i];
				if (ref.size != mesh.attribs.size) continue;
				bool match = true;
				for (uint32_t j = 0; j < ref.size; j++) {
					if (memcmp(&ref[j], &mesh.attribs[j], sizeof(spmdl_attrib)) != 0) {
						match = false;
						break;
					}
				}

				if (match) {
					vertexFormat = (VertexFormat)i;
					break;
				}
			}

		} else {
			// No meshes, nothing to render
		}

		if (vertexFormat == VertexFormat::Count) {
			// Early return: Invalid vertex format
			return;
		}

		model.vertexFormat = vertexFormat;

		model.modelBounds = model.model->bounds;
		if (model.shadowModel) {
			model.modelBounds = sf::boundsUnion(model.modelBounds, model.shadowModel->bounds);
		}
		model.worldBounds = sf::transformBounds(model.modelToWorld, model.modelBounds);

		if (model.areaId != ~0u) {
			areaSystem->updateBoxArea(model.areaId, model.worldBounds);
		} else {
			uint32_t areaFlags = Area::Visibility | Area::EditorPick;
			if (model.shadowModel) areaFlags |= Area::Shadow;
			model.areaId = areaSystem->addBoxArea(AreaGroup::DynamicModel, modelId, model.worldBounds, areaFlags);
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

	ModelSystemImp()
	{
		uint8_t permutation[SP_NUM_PERMUTATIONS] = { };
		#if CL_SHADOWCACHE_USE_ARRAY
			permutation[SP_SHADOWGRID_USE_ARRAY] = 1;
		#else
			permutation[SP_SHADOWGRID_USE_ARRAY] = 0;
		#endif
		permutation[SP_NORMALMAP_REMAP] = MeshMaterial::useNormalMapRemap;
		meshShader = getShader2(SpShader_DynamicMesh, permutation);


		uint32_t flags = sp::PipeDepthWrite|sp::PipeCullCCW|sp::PipeIndex16;

		{
			auto &d = meshPipes[(uint32_t)VertexFormat::Tile].init(meshShader.handle, flags);
			d.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
			d.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT3;
			d.layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT4;
			d.layout.attrs[3].format = SG_VERTEXFORMAT_FLOAT2;
		}

		{
			auto &d = meshPipes[(uint32_t)VertexFormat::Packed].init(meshShader.handle, flags);
			d.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
			d.layout.attrs[1].format = SG_VERTEXFORMAT_SHORT4N;
			d.layout.attrs[2].format = SG_VERTEXFORMAT_SHORT4N;
			d.layout.attrs[3].format = SG_VERTEXFORMAT_FLOAT2;
		}
	}

	sf::Box<void> preloadModel(const sv::DynamicModelComponent &c) override
	{
		auto data = sf::box<ModelData>();
		data->model.load(c.model);
		if (c.shadowModel) data->shadowModel.load(c.shadowModel);
		if (c.material) data->material.load(c.material);
		return data;
	}

	void addModel(Systems &systems, uint32_t entityId, uint8_t componentIndex, const sv::DynamicModelComponent &c, const Transform &transform) override
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
			if (c.shadowModel) {
				model.shadowModel.load(c.shadowModel);
			} else {
				model.shadowModel = model.model;
			}
		}
		model.material.load(c.material);

		startLoadingModel(systems.area, modelId);

		systems.entities.addComponent(entityId, this, modelId, 0, componentIndex, Entity::UpdateTransform);
	}

	void updateTransform(Systems &systems, uint32_t entityId, const EntityComponent &ec, const TransformUpdate &update) override
	{
		uint32_t modelId = ec.userId;
		Model &model = models[modelId];

		model.modelToWorld = update.entityToWorld * model.modelToEntity;
		model.worldBounds = sf::transformBounds(model.modelToWorld, model.modelBounds);

		if (model.areaId != ~0u) {
			systems.area->updateBoxArea(model.areaId, model.worldBounds);
		}
	}

	void remove(Systems &systems, uint32_t entityId, const EntityComponent &ec) override
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

	void renderMain(const LightSystem *lightSystem, const EnvLightSystem *envLightSystem, const VisibleAreas &visibleAreas, const RenderArgs &renderArgs) override
	{
		sf::SmallArray<cl::PointLight, 64> pointLights;
		const uint32_t maxLights = 16;

		UBO_DynamicTransform tu;
		UBO_Pixel pu;

		EnvLightAltas envLight = envLightSystem->getEnvLightAtlas();

		sg_bindings bindings = { };
		bindImageFS(meshShader, bindings, CL_SHADOWCACHE_TEX, lightSystem->getShadowTexture());
		bindImageFS(meshShader, bindings, TEX_diffuseEnvmapAtlas, lightSystem->getShadowTexture());

		for (uint32_t modelId : visibleAreas.get(AreaGroup::DynamicModel)) {
			Model &model = models[modelId];

			sg_image envmap = lightSystem->getEnvmapTexture(model.worldBounds);
			bindImageFS(meshShader, bindings, TEX_envmap, envmap);

			sp::Pipeline &pipe = meshPipes[(uint32_t)model.vertexFormat];
			tu.modelToWorld = model.modelToWorld;
			tu.worldToClip = renderArgs.worldToClip;

			sf::Bounds3 bounds = sf::transformBounds(model.modelToWorld, model.modelBounds);

			pointLights.clear();
			lightSystem->queryVisiblePointLights(visibleAreas, pointLights, bounds);

			// TODO: Prioritize
			if (pointLights.size > maxLights) {
				pointLights.resizeUninit(maxLights);
			}

			pu.numLightsF = (float)pointLights.size;
			pu.cameraPosition = renderArgs.cameraPosition;
			pu.diffuseEnvmapMad = envLight.worldMad;
			sf::Vec4 *dst = pu.pointLightData;
			for (PointLight &light : pointLights) {
				light.writeShader(dst);
			}

			pipe.bind();

			bindUniformVS(meshShader, tu);
			bindUniformFS(meshShader, pu);

			for (sp::Mesh &mesh : model.model->meshes) {

				sg_image *images = model.material ? model.material->images : MeshMaterial::defaultImages;
				bindImageFS(meshShader, bindings, TEX_albedoTexture, images[(uint32_t)MaterialTexture::Albedo]);
				bindImageFS(meshShader, bindings, TEX_normalTexture, images[(uint32_t)MaterialTexture::Normal]);
				bindImageFS(meshShader, bindings, TEX_maskTexture, images[(uint32_t)MaterialTexture::Mask]);

				bindings.index_buffer = model.model->indexBuffer.buffer;
				bindings.index_buffer_offset = mesh.indexBufferOffset;
				bindings.vertex_buffers[0] = model.model->vertexBuffer.buffer;
				bindings.vertex_buffer_offsets[0] = mesh.streams[0].offset;
				sg_apply_bindings(&bindings);

				sg_draw(0, mesh.numIndices, 1);
			}

		}
	}

	void editorHighlight(Systems &systems, const EntityComponent &ec, EditorHighlight type) override
	{
		uint32_t modelId = ec.userId;
		const Model &model = models[modelId];

		sf::Vec3 color;
		switch (type) {
		case EditorHighlight::Hover: color = sf::Vec3(0.3f, 0.2f, 0.2f); break;
		default: color = sf::Vec3(1.0f, 0.8f, 0.8f); break;
		}

		debugDrawBox(model.modelBounds, model.modelToWorld, color);
	}

	void editorPick(sf::Array<EntityHit> &hits, const sf::FastRay &ray, uint32_t userId) const override
	{
		uint32_t modelId = userId;
		const Model &model = models[modelId];
		if (!model.isLoaded()) return;

		float t = model.model->castModelRay(ray.ray, model.modelToWorld);
		if (t < HUGE_VALF) {
			hits.push({ model.entityId, t });
		}
	}

};

sf::Box<ModelSystem> ModelSystem::create() { return sf::box<ModelSystemImp>(); }

}
