#include "TileModelSystem.h"

#include "client/AreaSystem.h"
#include "client/TileMaterial.h"
#include "client/LightSystem.h"

#include "sf/Array.h"

#include "sp/Model.h"

#include "game/DebugDraw.h"

#include "game/shader/GameShaders.h"
#include "game/shader/MapShadow.h"

#include "game/shader2/GameShaders2.h"

namespace cl {

struct TileModelSystemImp final : TileModelSystem
{
	static const constexpr float ChunkSize = 8.0f;

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

	struct Model
	{
		uint32_t chunkId = ~0u;
		uint32_t indexInChunk = ~0u;

		uint32_t entityId;

		uint32_t loadQueueIndex = ~0u;
		sp::ModelRef model;
		sp::ModelRef shadowModel;
		cl::TileMaterialRef material;

		sf::Vec2i chunkPos;
		sf::Mat34 modelToEntity;
		sf::Mat34 modelToWorld;

		uint32_t tint = 0xffffffff;

		bool isLoading() const {
			return model.isLoading() && (shadowModel && shadowModel.isLoading()) && (material && material.isLoading());
		}
		bool isLoaded() const {
			return model.isLoaded() && (!shadowModel || shadowModel.isLoaded()) && (!material || material.isLoaded());
		}
	};

	struct Geometry
	{
		uint32_t areaId = ~0u;
		sp::Buffer vertexBuffer;
		sp::Buffer indexBuffer;
		sf::Bounds3 bounds;
		uint32_t numIndices = 0;
		bool largeIndices = false;

		void resetBuffers() {
			vertexBuffer.reset();
			indexBuffer.reset();
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
		bool loading = false;

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

		void finish(const char *indexName, Geometry &dst, AreaSystem *areaSystem, uint32_t chunkId, uint32_t areaFlags) {
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
			dst.bounds = bounds;

			if (dst.areaId == ~0u) {
				dst.areaId = areaSystem->addBoxArea(AreaGroup::TileChunk, chunkId, bounds, areaFlags);
			} else {
				areaSystem->updateBoxArea(dst.areaId, bounds);
			}
		}
	};

	struct Chunk
	{
		bool dirty = false;
		sf::Array<uint32_t> modelIds;
		sf::Vec2i chunkPos;
		Geometry main;
		Geometry shadow;
		bool uploaded = false;
		double lastVisibleTime = 0;
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
	sf::Array<uint32_t> freeChunkIdsNextFrame;

	sf::HashMap<sf::Vec2i, uint32_t> chunkMapping;

	sf::Array<uint32_t> loadQueue;

	uint32_t numUplodadedChunks = 0;
	uint32_t garbageCollectChunkIndex = 0;

	Shader2 chunkMeshShader;
	sp::Pipeline chunkMeshPipe[2];

	void updateChunkGeometry(uint32_t chunkId, AreaSystem *areaSystem)
	{
		Chunk &chunk = chunks[chunkId];

		GeometryBuilder mainBuilder, shadowBuilder;
		for (uint32_t &modelId : chunk.modelIds) {
			Model &model = models[modelId];
			mainBuilder.count(model.model);
			shadowBuilder.count(model.shadowModel);
		}

		chunk.main.resetBuffers();
		chunk.shadow.resetBuffers();

		mainBuilder.finishCount();
		shadowBuilder.finishCount();

		sf::Array<MapVertex> vertices;
		sf::Array<sf::Vec3> shadowVertices;
		vertices.resizeUninit(mainBuilder.numVertices);
		shadowVertices.resizeUninit(shadowBuilder.numVertices);
		MapVertex *vertexDst = vertices.data;
		sf::Vec3 *shadowVertexDst = shadowVertices.data;

		for (uint32_t modelId : chunk.modelIds) {
			Model &model = models[modelId];
			sf::Mat34 transform = model.modelToWorld;

			sf::Float4 col0 = sf::Float4::loadu(transform.cols[0].v).clearW();
			sf::Float4 col1 = sf::Float4::loadu(transform.cols[1].v).clearW();
			sf::Float4 col2 = sf::Float4::loadu(transform.cols[2].v).clearW();
			sf::Float4 col3 = sf::Float4::loadu(transform.cols[3].v - 1).rotateLeft().clearW();

			const constexpr float roundScale = 1000.0f;
			const constexpr float rcpRoundScale = 1.0f / roundScale;

			cl::TileMaterial *material = model.material;
			for (sp::Mesh &mesh : model.model->meshes) {
				sf_assert(mesh.streams[0].stride == sizeof(MapSrcVertex));
				mainBuilder.appendIndices(sf::slice(mesh.cpuIndexData16, mesh.numIndices), (uint32_t)(vertexDst - vertices.data));
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
					mainBuilder.updateBounds(tp);
				}
			}

			if (model.shadowModel) {
				for (sp::Mesh &mesh : model.shadowModel->meshes) {
					shadowBuilder.appendIndices(sf::slice(mesh.cpuIndexData16, mesh.numIndices), (uint32_t)(shadowVertexDst - shadowVertices.data));
					for (MapSrcVertex &vertex : sf::slice((MapSrcVertex*)mesh.streams[0].cpuData, mesh.numVertices)) {
						sf::Vec3 &dst = *shadowVertexDst++;

						const sf::Vec3 &vp = vertex.position;

						sf::Float4 tp = col0*vp.x + col1*vp.y + col2*vp.z + col3;

						tp = (tp * roundScale).round() * rcpRoundScale;

						dst = tp.asVec3();
						shadowBuilder.updateBounds(tp);
					}
				}
			}
		}

		sf::SmallStringBuf<256> name;
		name.format("MapChunk(%d,%d) ", chunk.chunkPos.x, chunk.chunkPos.y);
		uint32_t prefixLen = name.size;

		if (vertices.size) {
			name.resize(prefixLen); name.append(" vertices");
			chunk.main.vertexBuffer.initVertex(name.data, vertices.slice());
		}

		name.resize(prefixLen); name.append(" indices");
		mainBuilder.finish(name.data, chunk.main, areaSystem, chunkId, Area::Visibilty|Area::EditorPick);

		if (shadowVertices.size) {
			name.resize(prefixLen); name.append(" shadow vertices");
			chunk.shadow.vertexBuffer.initVertex(name.data, shadowVertices.slice());
		}

		name.resize(prefixLen); name.append(" shadow indices");
		shadowBuilder.finish(name.data, chunk.shadow, areaSystem, chunkId, Area::Shadow);
	}

	void addDirtyChunk(uint32_t chunkId)
	{
		Chunk &chunk = chunks[chunkId];
		chunk.dirty = true;
	}

	void addToChunkImp(AreaSystem *areaSystem, Model &model, uint32_t modelId)
	{
		auto res = chunkMapping.insert(model.chunkPos);
		if (res.inserted) {
			uint32_t chunkId = chunks.size;
			if (freeChunkIds.size > 0) {
				chunkId = freeChunkIds.popValue();
			} else {
				chunks.push();
			}

			Chunk &chunk = chunks[chunkId];
			chunk.chunkPos = model.chunkPos;
			res.entry.val = chunkId;
		}

		uint32_t chunkId = res.entry.val;
		Chunk &chunk = chunks[chunkId];
		model.chunkId = chunkId;
		model.indexInChunk = chunk.modelIds.size;
		chunk.modelIds.push(modelId);

		sf::Bounds3 bounds = sf::transformBounds(model.modelToWorld, model.model->bounds);
		if (chunk.modelIds.size > 1) {
			bounds = sf::boundsUnion(chunk.main.bounds, bounds);
		}

		if (bounds != chunk.main.bounds || chunk.modelIds.size == 1) {
			chunk.main.bounds = bounds;

			if (chunk.main.areaId == ~0u) {
				chunk.main.areaId = areaSystem->addBoxArea(AreaGroup::TileChunk, chunkId, bounds, Area::Visibilty|Area::EditorPick);
			} else {
				areaSystem->updateBoxArea(chunk.main.areaId, bounds);
			}
		}

		addDirtyChunk(chunkId);
	}

	void removeFromChunkImp(Model &model, uint32_t modelId, AreaSystem *areaSystem)
	{
		uint32_t chunkId = model.chunkId;
		Chunk &chunk = chunks[chunkId];
		if (chunk.modelIds.size > 1) {
			models[chunk.modelIds.back()].indexInChunk = model.indexInChunk;
			chunk.modelIds.removeSwap(model.indexInChunk);
			addDirtyChunk(chunkId);
		} else {
			if (chunk.main.areaId != ~0u) areaSystem->removeBoxArea(chunk.main.areaId);
			if (chunk.shadow.areaId != ~0u) areaSystem->removeBoxArea(chunk.shadow.areaId);
			chunkMapping.remove(chunk.chunkPos);
			freeChunkIdsNextFrame.push(chunkId);
			sf::reset(chunk);
		}
	}

	static sf::Vec2i getChunkFromPosition(const sf::Vec3 &pos)
	{
		return sf::Vec2i(sf::floor(sf::Vec2(pos.x, pos.z) * (1.0f / ChunkSize)));
	}

	void finishLoadingModel(AreaSystem *areaSystem, uint32_t modelId)
	{
		Model &model = models[modelId];
		addToChunkImp(areaSystem, model, modelId);
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

		model.chunkPos = getChunkFromPosition(transform.position);

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
		if (model.chunkId != ~0u) {
			if (chunkPos != model.chunkPos) {
				removeFromChunkImp(model, modelId, systems.area);
				model.chunkPos = chunkPos;
				addToChunkImp(systems.area, model, modelId);
			} else {
				addDirtyChunk(model.chunkId);
			}
		} else {
			model.chunkPos = chunkPos;
		}
	}

	void remove(Systems &systems, uint32_t entityId, const EntityComponent &ec) override
	{
		uint32_t modelId = ec.userId;
		Model &model = models[modelId];

		if (model.chunkId != ~0u) {
			removeFromChunkImp(model, modelId, systems.area);
		}

		freeModelIds.push(modelId);
		sf::reset(model);
	}

	void startFrame() override
	{
		freeChunkIds.push(freeChunkIdsNextFrame);
		freeChunkIdsNextFrame.clear();
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

	void uploadVisibleChunks(const VisibleAreas &visibleAreas, AreaSystem *areaSystem, const FrameArgs &frameArgs) override
	{
		for (uint32_t chunkId : visibleAreas.get(AreaGroup::TileChunk)) {
			Chunk &chunk = chunks[chunkId];

			chunk.lastVisibleTime = frameArgs.gameTime;

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

		double unloadTimer = 10.0;
		uint32_t numChunksToUpdate = 16;

		for (uint32_t i = 0; i < numChunksToUpdate; i++) {
			garbageCollectChunkIndex++;
			if (garbageCollectChunkIndex >= chunks.size) {
				garbageCollectChunkIndex = 0;
			}

			Chunk &chunk = chunks[garbageCollectChunkIndex];
			if (!chunk.uploaded) continue;

			if (frameArgs.gameTime - chunk.lastVisibleTime > unloadTimer) {
				if (chunk.main.areaId != ~0u) areaSystem->removeBoxArea(chunk.main.areaId);
				if (chunk.shadow.areaId != ~0u) areaSystem->removeBoxArea(chunk.shadow.areaId);
				chunk.main.resetBuffers();
				chunk.shadow.resetBuffers();
				chunk.uploaded = false;
			}
		}
	}

	void renderShadow(const VisibleAreas &shadowAreas, const RenderArgs &renderArgs) override
	{
		bool first = true;
		sg_bindings bindings = { };

		for (uint32_t chunkId : shadowAreas.get(AreaGroup::TileChunk)) {
			Chunk &chunk = chunks[chunkId];

			Geometry &geo = chunk.shadow;
			if (!geo.indexBuffer.buffer.id) continue;

			sp::Pipeline &pipe = gameShaders.mapChunkShadowPipe[geo.largeIndices];
			if (pipe.bind() || first) {
				first = false;
				MapShadow_Vertex_t vu;
				vu.cameraPosition = renderArgs.cameraPosition;
				renderArgs.worldToClip.writeColMajor44(vu.worldToClip);
				sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_MapShadow_Vertex, &vu, sizeof(vu));
			}

			bindings.vertex_buffers[0] = geo.vertexBuffer.buffer;
			bindings.index_buffer = geo.indexBuffer.buffer;
			sg_apply_bindings(&bindings);

			sg_draw(0, geo.numIndices, 1);
		}
	}

	void renderMain(const LightSystem *lightSystem, const VisibleAreas &visibleAreas, const RenderArgs &renderArgs) override
	{
		UBO_Transform tu = { };
		tu.worldToClip = renderArgs.worldToClip;

		UBO_Pixel pu = { };

		sf::SmallArray<cl::PointLight, 64> pointLights;

		const uint32_t maxLights = 16;

		sg_bindings bindings = { };
		bindImageFS(chunkMeshShader, bindings, CL_SHADOWCACHE_TEX, lightSystem->getShadowTexture());
		bindImageFS(chunkMeshShader, bindings, TEX_albedoAtlas, cl::TileMaterial::getAtlasImage(cl::MaterialTexture::Albedo));
		bindImageFS(chunkMeshShader, bindings, TEX_normalAtlas, cl::TileMaterial::getAtlasImage(cl::MaterialTexture::Normal));
		bindImageFS(chunkMeshShader, bindings, TEX_maskAtlas, cl::TileMaterial::getAtlasImage(cl::MaterialTexture::Mask));

		for (uint32_t chunkId : visibleAreas.get(AreaGroup::TileChunk)) {
			Chunk &chunk = chunks[chunkId];

			Geometry &geo = chunk.main;
			if (!geo.indexBuffer.buffer.id) continue;

			if (chunkMeshPipe[geo.largeIndices].bind()) {
				bindUniformVS(chunkMeshShader, tu);
			}

			pointLights.clear();
			lightSystem->queryVisiblePointLights(visibleAreas, pointLights, geo.bounds);

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

			bindUniformFS(chunkMeshShader, pu);

			bindings.vertex_buffers[0] = geo.vertexBuffer.buffer;
			bindings.index_buffer = geo.indexBuffer.buffer;

			sg_apply_bindings(&bindings);

			sg_draw(0, geo.numIndices, 1);
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
