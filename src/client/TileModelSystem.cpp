#include "TileModelSystem.h"

#include "client/AreaSystem.h"
#include "client/TileMaterial.h"
#include "client/LightSystem.h"
#include "client/EnvLightSystem.h"

#include "sf/Array.h"

#include "sp/Model.h"

#include "game/DebugDraw.h"

#include "game/shader/GameShaders.h"
#include "game/shader/MapShadow.h"
#include "game/shader/MapGBuffer.h"

#include "game/shader2/GameShaders2.h"

namespace cl {

struct TileModelSystemImp final : TileModelSystem
{
	static const constexpr float ChunkSize = 8.0f;

	static const constexpr float ChunkPadding = 16.0f;
	static const constexpr float ShadowPadding = 32.0f;

	struct MapSrcVertex
	{
		sf::Vec3 position;
		sf::Vec3 normal;
		sf::Vec4 tangent;
		sf::Vec2 uv;
	};

	struct MapVertex
	{
		sf::Vec3 position;
		sf::Vec3 normal;
		sf::Vec4 tangent;
		sf::Vec2 uv;
		uint32_t tint;
	};

	struct ChunkRef
	{
		uint32_t chunkId = ~0u;
		uint32_t indexInChunk = ~0u;
	};

	struct Model
	{
		sf::Vec2i chunkPos;
		ChunkRef chunks[2];

		uint32_t entityId;

		uint32_t loadQueueIndex = ~0u;
		sp::ModelRef model;
		sp::ModelRef shadowModel;
		cl::TileMaterialRef material;

		sf::Mat34 modelToEntity;
		sf::Mat34 modelToWorld;

		uint32_t tint = 0xffffffff;

		bool isLoading() const {
			return model.isLoading() || (shadowModel && shadowModel.isLoading()) || (material && material.isLoading());
		}
		bool isLoaded() const {
			return model.isLoaded() && (!shadowModel || shadowModel.isLoaded()) && (!material || material.isLoaded());
		}
	};

	struct Chunk
	{
		sf::Vec2i chunkPos;

		uint32_t cullingAreaId = ~0u;
		uint32_t activeAreaId = ~0u;

		sf::Array<uint32_t> modelIds;

		sp::Buffer vertexBuffer;
		sp::Buffer indexBuffer;
		sf::Bounds3 bounds;
		uint32_t numIndices = 0;
		bool largeIndices = false;

		bool dirty = false;
		bool uploaded = false;

		bool shadow = false;

		void resetBuffers() {
			vertexBuffer.reset();
			indexBuffer.reset();
		}

		void setBounds(uint32_t chunkId, AreaSystem *areaSystem, const sf::Bounds3 &newBounds) {

			bounds = newBounds;

			float padding;
			uint32_t areaFlags;
			if (shadow) {
				padding = ShadowPadding;
				areaFlags = Area::Shadow;
			} else {
				padding = ShadowPadding;
				areaFlags = Area::Visibility | Area::EditorPick | Area::Envmap;
			}

			if (cullingAreaId == ~0u) {
				cullingAreaId = areaSystem->addBoxArea(AreaGroup::TileChunkCulling, chunkId, newBounds, areaFlags);
			} else {
				areaSystem->updateBoxArea(cullingAreaId, newBounds);
			}

			sf::Bounds3 activeBounds = newBounds;
			activeBounds.extent += sf::Vec3(padding);

			if (activeAreaId == ~0u) {
				activeAreaId = areaSystem->addBoxArea(AreaGroup::TileChunkActive, chunkId, activeBounds, Area::Activate);
			} else {
				areaSystem->updateBoxArea(activeAreaId, activeBounds);
			}
		}

		void expandBounds(uint32_t chunkId, AreaSystem *areaSystem,const sf::Bounds3 &modelBounds) {
			sf::Bounds3 newBounds;
			if (modelIds.size == 1) {
				newBounds = modelBounds;
			} else {
				newBounds = sf::boundsUnion(bounds, modelBounds);
			}

			if (newBounds != bounds) {
				setBounds(chunkId, areaSystem, newBounds);
			}
		}
	};

	struct GeometryBuilder
	{
		uint32_t numVertices = 0, numIndices = 0;
		sf::Array<uint16_t> indices16;
		sf::Array<uint32_t> indices32;
		uint16_t *indicesDst16 = nullptr;
		uint32_t *indicesDst32 = nullptr;
		sf::Float4 aabbMin = sf::Float4(+HUGE_VALF), aabbMax = sf::Float4(-HUGE_VALF);

		void count(sp::Model *model) {
			if (!model) return;
			for (sp::Mesh &mesh : model->meshes) {
				numVertices += mesh.numVertices;
				numIndices += mesh.numIndices;
			}
		}

		void finishCount() {
			if (numIndices == 0) return;
			if (numVertices > UINT16_MAX) {
				indices32.resizeUninit(numIndices);
				indicesDst32 = indices32.data;
			} else {
				indices16.resizeUninit(numIndices);
				indicesDst16 = indices16.data;
			}
		}

		void appendIndices(sf::Slice<uint16_t> indices, uint32_t vertexOffset) {
			if (indicesDst16) {
				uint16_t *dst = indicesDst16;
				for (uint16_t index : indices) {
					*dst++ = (uint16_t)(vertexOffset + index);
				}
				indicesDst16 = dst;
			} else {
				uint32_t *dst = indicesDst32;
				for (uint16_t index : indices) {
					*dst++ = vertexOffset + index;
				}
				indicesDst32 = dst;
			}
		}

		sf_forceinline void updateBounds(const sf::Float4 &pos) {
			aabbMin = aabbMin.min(pos);
			aabbMax = aabbMax.max(pos);
		}

		void finish(const char *indexName, Chunk &dst, AreaSystem *areaSystem, uint32_t chunkId) {
			if (indicesDst16) {
				dst.indexBuffer.initIndex(indexName, indices16.slice());
				dst.largeIndices = false;
			} else if (indicesDst32) {
				dst.indexBuffer.initIndex(indexName, indices32.slice());
				dst.largeIndices = true;
			} else {
				dst.largeIndices = false;
			}
			dst.numIndices = numIndices;
			sf::Bounds3 bounds = sf::Bounds3::minMax(aabbMin.asVec3(), aabbMax.asVec3());
			dst.setBounds(chunkId, areaSystem, bounds);
		}
	};

	static sf::Mat34 getComponentTransform(const sv::TileModelComponent &c)
	{
		return sf::mat::translate(c.position) * (
		sf::mat::rotateZ(c.rotation.z * (sf::F_PI/180.0f)) *
		sf::mat::rotateY(c.rotation.y * (sf::F_PI/180.0f)) *
		sf::mat::rotateX(c.rotation.x * (sf::F_PI/180.0f)) *
		sf::mat::scale(c.stretch * c.scale * 0.01f));
	}

	sf::Array<Model> models;
	sf::Array<uint32_t> freeModelIds;

	sf::Array<Chunk> chunks;
	sf::Array<uint32_t> freeChunkIds;

	sf::HashMap<sf::Vec2i, uint32_t> chunkMapping[2];
	sf::HashSet<uint32_t> emptyChunks;

	sf::Array<uint32_t> loadQueue;

	uint32_t numUplodadedChunks = 0;
	uint32_t garbageCollectChunkIndex = 0;

	Shader2 chunkMeshShader;
	sp::Pipeline chunkMeshPipe[2];

	void updateChunkGeometry(uint32_t chunkId, AreaSystem *areaSystem)
	{
		Chunk &chunk = chunks[chunkId];

		GeometryBuilder builder;
		for (uint32_t &modelId : chunk.modelIds) {
			Model &model = models[modelId];
			builder.count(chunk.shadow ? model.shadowModel : model.model);
		}

		chunk.resetBuffers();

		builder.finishCount();

		const constexpr float roundScale = 1000.0f;
		const constexpr float rcpRoundScale = 1.0f / roundScale;

		sf::SmallStringBuf<256> name;
		if (chunk.shadow) {
			name.format("ShadowChunk(%d,%d) ", chunk.chunkPos.x, chunk.chunkPos.y);
		} else {
			name.format("MapChunk(%d,%d) ", chunk.chunkPos.x, chunk.chunkPos.y);
		}
		uint32_t prefixLen = name.size;

		if (chunk.shadow) {
			sf::Array<sf::Vec3> vertices;
			vertices.resizeUninit(builder.numVertices);
			sf::Vec3 *vertexDst = vertices.data;

			for (uint32_t modelId : chunk.modelIds) {
				Model &model = models[modelId];
				sf::Mat34 transform = model.modelToWorld;

				sf::Float4 col0 = sf::Float4::loadu(transform.cols[0].v).clearW();
				sf::Float4 col1 = sf::Float4::loadu(transform.cols[1].v).clearW();
				sf::Float4 col2 = sf::Float4::loadu(transform.cols[2].v).clearW();
				sf::Float4 col3 = sf::Float4::loadu(transform.cols[3].v - 1).rotateLeft().clearW();

				for (sp::Mesh &mesh : model.shadowModel->meshes) {
					builder.appendIndices(sf::slice(mesh.cpuIndexData16, mesh.numIndices), (uint32_t)(vertexDst - vertices.data));
					for (MapSrcVertex &vertex : sf::slice((MapSrcVertex*)mesh.streams[0].cpuData, mesh.numVertices)) {
						sf::Vec3 &dst = *vertexDst++;

						const sf::Vec3 &vp = vertex.position;

						sf::Float4 tp = col0*vp.x + col1*vp.y + col2*vp.z + col3;

						tp = (tp * roundScale).round() * rcpRoundScale;

						dst = tp.asVec3();
						builder.updateBounds(tp);
					}
				}
			}

			if (vertices.size) {
				name.resize(prefixLen); name.append(" vertices");
				chunk.vertexBuffer.initVertex(name.data, vertices.slice());
			}

		} else {
			sf::Array<MapVertex> vertices;
			vertices.resizeUninit(builder.numVertices);
			MapVertex *vertexDst = vertices.data;

			for (uint32_t modelId : chunk.modelIds) {
				Model &model = models[modelId];
				sf::Mat34 transform = model.modelToWorld;

				sf::Float4 col0 = sf::Float4::loadu(transform.cols[0].v).clearW();
				sf::Float4 col1 = sf::Float4::loadu(transform.cols[1].v).clearW();
				sf::Float4 col2 = sf::Float4::loadu(transform.cols[2].v).clearW();
				sf::Float4 col3 = sf::Float4::loadu(transform.cols[3].v - 1).rotateLeft().clearW();

				cl::TileMaterial *material = model.material;
				for (sp::Mesh &mesh : model.model->meshes) {
					sf_assert(mesh.streams[0].stride == sizeof(MapSrcVertex));
					builder.appendIndices(sf::slice(mesh.cpuIndexData16, mesh.numIndices), (uint32_t)(vertexDst - vertices.data));
					for (MapSrcVertex &vertex : sf::slice((MapSrcVertex*)mesh.streams[0].cpuData, mesh.numVertices)) {
						MapVertex &dst = *vertexDst++;
						const sf::Vec3 &vp = vertex.position;
						const sf::Vec3 &vn = vertex.normal;
						const sf::Vec4 &vt = vertex.tangent;

						sf::Float4 tp = col0*vp.x + col1*vp.y + col2*vp.z + col3;
						sf::Float4 tn = col0*vn.x + col1*vn.y + col2*vn.z;
						sf::Float4 tt = col0*vt.x + col1*vt.y + col2*vt.z;

						tp = (tp * roundScale).round() * rcpRoundScale;
						tn *= sf::broadcastRcpLengthXYZ(tn);
						tt *= sf::broadcastRcpLengthXYZ(tt);

						dst.position = tp.asVec3();
						dst.normal = tn.asVec3();
						dst.tangent = sf::Vec4(tt.asVec3(), vertex.tangent.w);
						dst.uv = vertex.uv * material->uvScale + material->uvBase;
						dst.tint = model.tint;
						builder.updateBounds(tp);
					}
				}
			}

			if (vertices.size) {
				name.resize(prefixLen); name.append(" vertices");
				chunk.vertexBuffer.initVertex(name.data, vertices.slice());
			}
		}

		name.resize(prefixLen); name.append(" indices");
		builder.finish(name.data, chunk, areaSystem, chunkId);
	}

	void addDirtyChunk(uint32_t chunkId)
	{
		Chunk &chunk = chunks[chunkId];
		chunk.dirty = true;
	}

	void addToChunkImp(AreaSystem *areaSystem, Model &model, uint32_t modelId, const sf::Vec2i &chunkPos, bool shadow)
	{
		auto res = chunkMapping[shadow].insert(chunkPos);
		if (res.inserted) {
			uint32_t chunkId = chunks.size;
			if (freeChunkIds.size > 0) {
				chunkId = freeChunkIds.popValue();
			} else {
				chunks.push();
			}

			Chunk &chunk = chunks[chunkId];
			chunk.chunkPos = chunkPos;
			chunk.shadow = shadow;
			res.entry.val = chunkId;
		}

		uint32_t chunkId = res.entry.val;
		Chunk &chunk = chunks[chunkId];
		sf_assert(chunk.shadow == shadow);

		model.chunks[shadow].chunkId = chunkId;
		model.chunks[shadow].indexInChunk = chunk.modelIds.size;
		chunk.modelIds.push(modelId);

		sf::Bounds3 bounds = sf::transformBounds(model.modelToWorld, model.model->bounds);
		chunk.expandBounds(chunkId, areaSystem, bounds);

		addDirtyChunk(chunkId);
	}

	void removeFromChunkImp(Model &model, uint32_t modelId, AreaSystem *areaSystem, bool shadow)
	{
		uint32_t chunkId = model.chunks[shadow].chunkId;
		uint32_t indexInChunk = model.chunks[shadow].indexInChunk;
		Chunk &chunk = chunks[chunkId];
		sf_assert(chunk.modelIds[indexInChunk] == modelId);
		sf_assert(chunk.shadow == shadow);
		if (chunk.modelIds.size > 1) {
			models[chunk.modelIds.back()].chunks[shadow].indexInChunk = indexInChunk;
			chunk.modelIds.removeSwap(indexInChunk);
			addDirtyChunk(chunkId);
		} else {
			chunk.modelIds.clear();
			emptyChunks.insert(chunkId);
		}
	}

	static sf::Vec2i getChunkFromPosition(const sf::Vec3 &pos)
	{
		return sf::Vec2i(sf::floor(sf::Vec2(pos.x, pos.z) * (1.0f / ChunkSize)));
	}

	static sf::Vec2i getShadowChunkFromChunk(const sf::Vec2i &chunk)
	{
		return sf::Vec2i(chunk.x >> 1, chunk.y >> 1);
	}

	void finishLoadingModel(AreaSystem *areaSystem, uint32_t modelId)
	{
		Model &model = models[modelId];

		if (model.model.isLoaded()) {
			addToChunkImp(areaSystem, model, modelId, model.chunkPos, false);
		}

		if (model.shadowModel.isLoaded()) {
			sf::Vec2i shadowPos = getShadowChunkFromChunk(model.chunkPos);
			addToChunkImp(areaSystem, model, modelId, shadowPos, true);
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

	TileModelSystemImp()
	{
		uint8_t permutation[SP_NUM_PERMUTATIONS] = { };
		#if CL_SHADOWCACHE_USE_ARRAY
			permutation[SP_SHADOWGRID_USE_ARRAY] = 1;
		#else
			permutation[SP_SHADOWGRID_USE_ARRAY] = 0;
		#endif
		permutation[SP_NORMALMAP_REMAP] = MeshMaterial::useNormalMapRemap;
		chunkMeshShader = getShader2(SpShader_TestMesh, permutation);

		for (uint32_t ix = 0; ix < 2; ix++) {
			uint32_t flags = sp::PipeDepthWrite|sp::PipeCullCCW;
			flags |= ix == 1 ? sp::PipeIndex32 : sp::PipeIndex16;
			auto &d = chunkMeshPipe[ix].init(chunkMeshShader.handle, flags);
			d.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
			d.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT3;
			d.layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT4;
			d.layout.attrs[3].format = SG_VERTEXFORMAT_FLOAT2;
			d.layout.attrs[4].format = SG_VERTEXFORMAT_UBYTE4N;
		}
	}

	void addModel(Systems &systems, uint32_t entityId, uint8_t componentIndex, const sv::TileModelComponent &c, const Transform &transform) override
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

		sf::Vec2i chunkPos = getChunkFromPosition(transform.position);
		model.chunkPos = chunkPos;

		sp::ModelProps modelProps;
		modelProps.cpuData = true;

		model.model.load(c.model, modelProps);
		if (c.castShadows) {
			if (c.shadowModel) {
				model.shadowModel.load(c.shadowModel, modelProps);
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

		sf::Vec2i chunkPos = getChunkFromPosition(update.transform.position);

		if (model.chunks[0].chunkId != ~0u) {
			if (chunkPos != model.chunkPos) {
				removeFromChunkImp(model, modelId, systems.area, false);
				addToChunkImp(systems.area, model, modelId, chunkPos, false);
			} else {
				addDirtyChunk(model.chunks[0].chunkId);
			}
		}

		if (model.chunks[1].chunkId != ~0u) {
			sf::Vec2i shadowPos = getShadowChunkFromChunk(chunkPos);

			if (shadowPos != getShadowChunkFromChunk(model.chunkPos)) {
				removeFromChunkImp(model, modelId, systems.area, true);
				addToChunkImp(systems.area, model, modelId, shadowPos, true);
			} else {
				addDirtyChunk(model.chunks[1].chunkId);
			}
		}

		model.chunkPos = chunkPos;
	}

	void remove(Systems &systems, uint32_t entityId, const EntityComponent &ec) override
	{
		uint32_t modelId = ec.userId;
		Model &model = models[modelId];

		if (model.chunks[0].chunkId != ~0u) {
			removeFromChunkImp(model, modelId, systems.area, false);
		}
		if (model.chunks[1].chunkId != ~0u) {
			removeFromChunkImp(model, modelId, systems.area, true);
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

	void uploadVisibleChunks(const VisibleAreas &activeAreas, AreaSystem *areaSystem, const FrameArgs &frameArgs) override
	{
		for (uint32_t chunkId : activeAreas.get(AreaGroup::TileChunkActive)) {
			Chunk &chunk = chunks[chunkId];
			if (chunk.modelIds.size == 0) continue;

			if (!chunk.uploaded || chunk.dirty) {
				updateChunkGeometry(chunkId, areaSystem);
				chunk.uploaded = true;
				chunk.dirty = false;
			}
		}
	}

	void garbageCollectChunks(AreaSystem *areaSystem, const FrameArgs &frameArgs) override
	{
		if (chunks.size == 0) return;

		for (uint32_t i = 0; i < emptyChunks.size(); i++) {
			uint32_t chunkId = emptyChunks.data[i];
			Chunk &chunk = chunks[chunkId];

			if (chunk.modelIds.size == 0) {
				if (chunk.cullingAreaId != ~0u) areaSystem->removeBoxArea(chunk.cullingAreaId);
				if (chunk.activeAreaId != ~0u) areaSystem->removeBoxArea(chunk.activeAreaId);

				chunkMapping[chunk.shadow].remove(chunk.chunkPos);
				freeChunkIds.push(chunkId);
				emptyChunks.remove(chunkId);
				sf::reset(chunk);
				i--;
			}
		}
	}

	void renderShadow(const VisibleAreas &shadowAreas, const RenderArgs &renderArgs) override
	{
		bool first = true;
		sg_bindings bindings = { };

		for (uint32_t chunkId : shadowAreas.get(AreaGroup::TileChunkCulling)) {
			Chunk &chunk = chunks[chunkId];
			sf_assert(chunk.shadow);
			if (!chunk.indexBuffer.buffer.id) continue;

			sp::Pipeline &pipe = gameShaders.mapChunkShadowPipe[chunk.largeIndices];
			if (pipe.bind() || first) {
				first = false;
				MapShadow_Vertex_t vu;
				vu.cameraPosition = renderArgs.cameraPosition;
				renderArgs.worldToClip.writeColMajor44(vu.worldToClip);
				sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_MapShadow_Vertex, &vu, sizeof(vu));
			}

			bindings.vertex_buffers[0] = chunk.vertexBuffer.buffer;
			bindings.index_buffer = chunk.indexBuffer.buffer;
			sg_apply_bindings(&bindings);

			sg_draw(0, chunk.numIndices, 1);
		}
	}

	void renderMain(const LightSystem *lightSystem, const EnvLightSystem *envLightSystem, const VisibleAreas &visibleAreas, const RenderArgs &renderArgs) override
	{
		UBO_Transform tu = { };
		tu.worldToClip = renderArgs.worldToClip;

		UBO_Pixel pu = { };

		sf::SmallArray<cl::PointLight, 64> pointLights;

		const uint32_t maxLights = 16;

		EnvLightAltas envLight = envLightSystem->getEnvLightAtlas();

		sg_bindings bindings = { };
		bindImageFS(chunkMeshShader, bindings, CL_SHADOWCACHE_TEX, lightSystem->getShadowTexture());
		bindImageFS(chunkMeshShader, bindings, TEX_albedoAtlas, cl::TileMaterial::getAtlasImage(cl::MaterialTexture::Albedo));
		bindImageFS(chunkMeshShader, bindings, TEX_normalAtlas, cl::TileMaterial::getAtlasImage(cl::MaterialTexture::Normal));
		bindImageFS(chunkMeshShader, bindings, TEX_maskAtlas, cl::TileMaterial::getAtlasImage(cl::MaterialTexture::Mask));
		bindImageFS(chunkMeshShader, bindings, TEX_diffuseEnvmapAtlas, envLight.image);

		for (uint32_t chunkId : visibleAreas.get(AreaGroup::TileChunkCulling)) {
			Chunk &chunk = chunks[chunkId];
			sf_assert(!chunk.shadow);
			if (!chunk.indexBuffer.buffer.id) continue;

			if (chunkMeshPipe[chunk.largeIndices].bind()) {
				bindUniformVS(chunkMeshShader, tu);
			}

			sg_image envmap = lightSystem->getEnvmapTexture(chunk.bounds);
			bindImageFS(chunkMeshShader, bindings, TEX_envmap, envmap);

			pointLights.clear();
			lightSystem->queryVisiblePointLights(visibleAreas, pointLights, chunk.bounds);

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

			bindUniformFS(chunkMeshShader, pu);

			bindings.vertex_buffers[0] = chunk.vertexBuffer.buffer;
			bindings.index_buffer = chunk.indexBuffer.buffer;

			sg_apply_bindings(&bindings);

			sg_draw(0, chunk.numIndices, 1);
		}
	}

	void renderEnvmapGBuffer(const VisibleAreas &envmapAreas, const RenderArgs &renderArgs) override
	{
		bool first = true;
		sg_bindings bindings = { };

		sg_image albedoAltas = cl::TileMaterial::getAtlasImage(cl::MaterialTexture::Albedo);
		bindings.fs_images[SLOT_MapGBuffer_albedoAtlas] = albedoAltas;

		for (uint32_t chunkId : envmapAreas.get(AreaGroup::TileChunkCulling)) {
			Chunk &chunk = chunks[chunkId];
			sf_assert(!chunk.shadow);
			if (!chunk.indexBuffer.buffer.id) continue;

			sp::Pipeline &pipe = gameShaders.mapChunkEnvmapPipe[chunk.largeIndices];
			if (pipe.bind() || first) {
				first = false;
				MapGBuffer_Vertex_t vu;
				renderArgs.worldToClip.writeColMajor44(vu.worldToClip);
				sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_MapShadow_Vertex, &vu, sizeof(vu));
			}

			bindings.vertex_buffers[0] = chunk.vertexBuffer.buffer;
			bindings.index_buffer = chunk.indexBuffer.buffer;
			sg_apply_bindings(&bindings);

			sg_draw(0, chunk.numIndices, 1);
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

		debugDrawBox(model.model->bounds, model.modelToWorld, color);
	}

	void editorPick(sf::Array<EntityHit> &hits, const sf::FastRay &ray, uint32_t userId) const override
	{
		uint32_t chunkId = userId;
		const Chunk &chunk = chunks[chunkId];

		for (uint32_t modelId : chunk.modelIds) {
			const Model &model = models[modelId];
			float t = model.model->castModelRay(ray.ray, model.modelToWorld);
			if (t < HUGE_VALF) {
				hits.push({ model.entityId, t });
			}
		}
	}

};

sf::Box<TileModelSystem> TileModelSystem::create() { return sf::box<TileModelSystemImp>(); }

}
