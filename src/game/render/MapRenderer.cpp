#if 0
#include "MapRenderer.h"

#include "sf/Array.h"
#include "sf/HashMap.h"
#include "sp/Model.h"
#include "game/Game.h"
#include "ext/sokol/sokol_gfx.h"
#include "game/shader/MapTile.h"
#include "game/shader/LightGrid.h"
#include "game/shader/MapShadow.h"
#include "game/shader/GameShaders.h"
#include "sp/Renderer.h"
#include "sf/Geometry.h"
#include "sf/Frustum.h"

// TEMP TEMP
#include "ext/sokol/sokol_time.h"
#include "ext/sokol/sokol_gl.h"

struct ChunkModel
{
	sp::ModelRef model;
	sp::ModelRef shadowModel;
	sf::Vec2i tile;
	sf::Mat34 transform;
	Entity entity;
};

struct MapMesh
{
	sf::Bounds3 bounds;
	uint32_t numVertices = 0;
	uint32_t numIndices = 0;
	sg_buffer vertexBuffer = { 0 };
	sg_buffer indexBuffer = { 0 };
	bool largeIndices = false;
};

struct MapChunk
{
	bool dirty = false;
	sf::Vec2i chunkI;
	sf::Array<ChunkModel> models;

	MapMesh mesh;
	MapMesh shadowMesh;
};

struct MapRenderer::Data
{
	static const uint32_t ChunkSizeLog2 = 3;

	sf::HashMap<sf::Vec2i, MapChunk> chunks;
	sf::Array<sf::Vec2i> dirtyChunks;

	sp::Pipeline pipeMapTile[2 /* largeIndices */];

	void setChunkDirty(MapChunk &chunk)
	{
		if (chunk.dirty) return;
		chunk.dirty = true;
		dirtyChunks.push(chunk.chunkI);
	}

	MapChunk &getChunk(const sf::Vec2i &tile)
	{
		sf::Vec2i chunkI { tile.x >> (int32_t)ChunkSizeLog2, tile.y >> (int32_t)ChunkSizeLog2 };
		auto res = chunks.insert(chunkI);
		if (res.inserted) res.entry.val.chunkI = chunkI;
		return res.entry.val;
	}

	MapTile_LightGrid_t testPixel;
	sp::RenderTarget testLightGridImage;
	sp::RenderPass testShadowPass;
	sp::RenderPass testLightPass;

	sp::Pipeline testLightPipe;
	LightGrid_Vertex_t testLightVertex;
	LightGrid_Pixel_t testLightPixel;

	uint32_t testShadowExtent;
	uint32_t testShadowAtlasWidth, testShadowAtlasHeight;
	sp::RenderTarget testShadowAtlas;
	sp::RenderTarget testShadowDepth;
	sp::Pipeline testShadowPipe[2];
	sf::Vec3 testShadowOrigin;
	sf::Vec3 testShadowRcpScale;
	float testShadowYSlices;
	sg_buffer shadowResetVertexBuffer;
	sg_buffer shadowResetIndexBuffer;

	sg_buffer testPostVertexBuffer;

	sp::Pipeline testShadowResetPipe;
};

sg_buffer g_hackPostVertexBuffer;

struct TestLight
{
	sf::Vec3 position;
	float radius;
	sf::Vec3 color;
	sf::Vec4 shadowMad;
};

MapRenderer::MapRenderer()
	: data(new Data())
{
	for (int largeIndices = 0; largeIndices < 2; largeIndices++)
	{
		uint32_t flags = sp::PipeDepthWrite | sp::PipeCullCCW;
		sg_pipeline_desc &d = data->pipeMapTile[largeIndices].init(gameShaders.mapTile, flags);
		d.index_type = largeIndices ? SG_INDEXTYPE_UINT32 : SG_INDEXTYPE_UINT16;
		d.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
		d.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT3;
		d.layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT2;
		d.label = "mapTile";
	}

	{
		sf::Vec2 postVerts[] = {
			{ 0.0f, 0.0f },
			{ 2.0f, 0.0f },
			{ 0.0f, 2.0f },
		};

		sg_buffer_desc d = { };
		d.type = SG_BUFFERTYPE_VERTEXBUFFER;
		d.content = postVerts;
		d.size = sizeof(postVerts);
		data->testPostVertexBuffer = sg_make_buffer(&d);
		g_hackPostVertexBuffer = data->testPostVertexBuffer;
	}

	{
		const uint32_t Extent = 128;
		const uint32_t Slices = 8;
		const uint32_t TexWidth = Extent*Slices;
		const uint32_t TexHeight = Extent*7;

		float scale = 32.0f;
		sf::Vec3 origin = sf::Vec3(-scale-0.5f, 0.0f, -scale-0.5f);
		sf::Vec3 size = sf::Vec3(scale*2.0f-1.0f, 4.0f-1.0f, scale*2.0f-1.0f);

		data->testLightVertex.lightGridYSlices = (float)Slices;
		data->testLightPixel.lightGridOrigin = origin;
		data->testLightPixel.lightGridScale = size / sf::Vec3(1.0f, (float)Slices, 1.0f);

		data->testPixel.lightGridOrigin = origin;
		data->testPixel.lightGridRcpScale = sf::Vec3(1.0f) / size;
		data->testPixel.lightGridYSlices = (float)Slices;
		data->testPixel.lightGridRcpYSlices = 1.0f / (float)Slices;

		sf::Vec2i res { (int)TexWidth, (int)TexHeight };

		data->testLightGridImage.init("lightTestGrid", res, SG_PIXELFORMAT_RG11B10F);
		data->testLightPass.init("lightGrid", data->testLightGridImage);
	}

	{
		const uint32_t ShadowExtent = 128;
		const uint32_t ShadowNumX = 4;
		const uint32_t ShadowNumY = 32;

		const uint32_t TexWidth = ShadowExtent*6*ShadowNumX;
		const uint32_t TexHeight = ShadowExtent*ShadowNumY;

		data->testShadowExtent = ShadowExtent;
		data->testShadowAtlasWidth = 6*ShadowNumX;
		data->testShadowAtlasHeight = ShadowNumY;

		sf::Vec2i res { (int)TexWidth, (int)TexHeight };

		{
			sg_image_desc d = { };
			d.min_filter = d.mag_filter = SG_FILTER_NEAREST;
			data->testShadowAtlas.init("shadowAtlas", res, SG_PIXELFORMAT_R16F, 1u, d);
		}

		{
			data->testShadowDepth.init("shadowDepth", res, SG_PIXELFORMAT_DEPTH);
		}

		data->testShadowPass.init("shadow", data->testShadowAtlas, data->testShadowDepth);

		for (int largeIndices = 0; largeIndices < 2; largeIndices++) {
			uint32_t flags = sp::PipeDepthWrite | sp::PipeCullAuto;
			sg_pipeline_desc &d = data->testShadowPipe[largeIndices].init(gameShaders.mapShadow, flags);
			d.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
			d.index_type = largeIndices ? SG_INDEXTYPE_UINT32 : SG_INDEXTYPE_UINT16;
		}

		{
			sg_pipeline_desc &d = data->testShadowResetPipe.init(gameShaders.mapShadow, sp::PipeCullAuto|sp::PipeIndex16);
			d.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
			d.depth_stencil.depth_write_enabled = true;
			d.depth_stencil.depth_compare_func = SG_COMPAREFUNC_ALWAYS;
		}
	}

	data->testLightPipe.init(gameShaders.lightGrid, sp::PipeVertexFloat2 | sp::PipeBlendAdd);

	{
		sf::Vec3 shadowResetVerts[] = {
			{ -1.0f, -1.0f, 1.0f },
			{ +1.0f, -1.0f, 1.0f },
			{ -1.0f, +1.0f, 1.0f },
			{ +1.0f, +1.0f, 1.0f },
		};
		sg_buffer_desc d = { };
		d.type = SG_BUFFERTYPE_VERTEXBUFFER;
		d.content = shadowResetVerts;
		d.size = sizeof(shadowResetVerts);
		d.label = "shadowResetVertexBuffe";
		data->shadowResetVertexBuffer = sg_make_buffer(&d);
	}

	{
		uint16_t shadowResetIndices[] = {
			0, 1, 2, 2, 1, 3,
		};
		sg_buffer_desc d = { };
		d.type = SG_BUFFERTYPE_INDEXBUFFER;
		d.content = shadowResetIndices;
		d.size = sizeof(shadowResetIndices);
		d.label = "shadowResetIndexBuffer";
		data->shadowResetIndexBuffer = sg_make_buffer(&d);
	}

}

MapRenderer::~MapRenderer()
{
	delete data;
}

void MapRenderer::addMapModel(Entity e, const MapModel &model)
{
	sf::Vec2i tile = t_game->map.getTile(e);
	MapChunk &chunk = data->getChunk(tile);
	data->setChunkDirty(chunk);

	sp::ModelProps props;
	props.cpuData = true;

	ChunkModel &chunkModel = chunk.models.push();
	chunkModel.entity = e;
	chunkModel.model.load(model.modelName, props);
	if (model.shadowModelName) {
		chunkModel.shadowModel.load(model.shadowModelName, props);
	}
	chunkModel.tile = tile;
	chunkModel.transform = model.transform;
}

void MapRenderer::removeMapModel(Entity e)
{
	sf::Vec2i tile = t_game->map.getTile(e);
	MapChunk &chunk = data->getChunk(tile);
	data->setChunkDirty(chunk);

	for (ChunkModel &model : chunk.models) {
		if (model.entity == e) {
			chunk.models.removeSwapPtr(&model);
			break;
		}
	}
}

static bool rebuildChunk(MapRenderer::Data *data, MapChunk &chunk)
{
	uint32_t numVertices = 0;
	uint32_t numIndices = 0;
	uint32_t shadowNumVertices = 0;
	uint32_t shadowNumIndices = 0;

	for (ChunkModel &model : chunk.models) {
		if (!model.model.isLoaded()) return false;
		if (model.shadowModel && !model.shadowModel.isLoaded()) return false;

		for (sp::Mesh &mesh : model.model->meshes) {
			numVertices += mesh.numVertices;
			numIndices += mesh.numIndices;
			if (model.shadowModel) {
				shadowNumVertices += mesh.numVertices;
				shadowNumIndices += mesh.numIndices;
			}
		}
	}

	sg_destroy_buffer(chunk.mesh.vertexBuffer);
	sg_destroy_buffer(chunk.mesh.indexBuffer);
	sg_destroy_buffer(chunk.shadowMesh.vertexBuffer);
	sg_destroy_buffer(chunk.shadowMesh.indexBuffer);
	chunk.mesh.vertexBuffer = { };
	chunk.mesh.indexBuffer = { };
	chunk.shadowMesh.vertexBuffer = { };
	chunk.shadowMesh.indexBuffer = { };

	sf::Array<sp::Vertex> vertices;
	sf::Array<uint16_t> indices16;
	sf::Array<uint32_t> indices32;
	uint16_t *indexDst16 = NULL;
	uint32_t *indexDst32 = NULL;
	vertices.resizeUninit(numVertices);
	if (numVertices > UINT16_MAX) {
		indices32.resizeUninit(numIndices);
		indexDst32 = indices32.data;
		chunk.mesh.largeIndices = true;
	} else {
		indices16.resizeUninit(numIndices);
		indexDst16 = indices16.data;
		chunk.mesh.largeIndices = false;
	}
	sp::Vertex *vertexDst = vertices.data;

	sf::Array<sf::Vec3> shadowVertices;
	sf::Array<uint16_t> shadowIndices16;
	sf::Array<uint32_t> shadowIndices32;
	uint16_t *shadowIndexDst16 = NULL;
	uint32_t *shadowIndexDst32 = NULL;
	shadowVertices.resizeUninit(shadowNumVertices);
	if (shadowNumVertices > UINT16_MAX) {
		shadowIndices32.resizeUninit(shadowNumIndices);
		shadowIndexDst32 = shadowIndices32.data;
		chunk.shadowMesh.largeIndices = true;
	} else {
		shadowIndices16.resizeUninit(shadowNumIndices);
		shadowIndexDst16 = shadowIndices16.data;
		chunk.shadowMesh.largeIndices = false;
	}
	sf::Vec3 *shadowVertexDst = shadowVertices.data;

	sf::Vec3 aabbMin = sf::Vec3(+HUGE_VALF);
	sf::Vec3 aabbMax = sf::Vec3(-HUGE_VALF);
	sf::Vec3 shadowAabbMin = sf::Vec3(+HUGE_VALF);
	sf::Vec3 shadowAabbMax = sf::Vec3(-HUGE_VALF);

	uint32_t vertexOffset = 0;
	uint32_t shadowVertexOffset = 0;
	for (ChunkModel &model : chunk.models) {

		for (sp::Mesh &mesh : model.model->meshes) {

			sf::Mat34 transform = sf::mat::translate((float)model.tile.x, 0.0f, (float)model.tile.y) * model.transform;
			// TODO: sf::Mat33 normalTransform = sf::transpose(sf::inverse(transform.get33()));
			sf::Mat33 normalTransform = transform.get33();

			for (sp::Vertex &vertex : mesh.vertexData) {
				sp::Vertex &dst = *vertexDst++;
				dst.position = sf::transformPoint(transform, vertex.position);
				dst.normal = sf::normalizeOrZero(sf::transformPoint(normalTransform, vertex.normal));
				dst.uv = vertex.uv;

				aabbMin = sf::min(aabbMin, dst.position);
				aabbMax = sf::max(aabbMax, dst.position);
			}

			if (indexDst16) {
				for (uint16_t index : mesh.indexData) {
					*indexDst16++ = (uint16_t)(vertexOffset + index);
				}
			} else {
				for (uint16_t index : mesh.indexData) {
					*indexDst32++ = vertexOffset + index;
				}
			}

			vertexOffset += mesh.numVertices;
		}

		if (model.shadowModel) {
			for (sp::Mesh &mesh : model.shadowModel->meshes) {

				sf::Mat34 transform = sf::mat::translate((float)model.tile.x, 0.0f, (float)model.tile.y) * model.transform;

				for (sp::Vertex &vertex : mesh.vertexData) {
					sf::Vec3 &dst = *shadowVertexDst++;
					dst = sf::transformPoint(transform, vertex.position);

					shadowAabbMin = sf::min(shadowAabbMin, dst);
					shadowAabbMax = sf::max(shadowAabbMax, dst);
				}

				if (shadowIndexDst16) {
					for (uint16_t index : mesh.indexData) {
						*shadowIndexDst16++ = (uint16_t)(shadowVertexOffset + index);
					}
				} else {
					for (uint16_t index : mesh.indexData) {
						*shadowIndexDst32++ = shadowVertexOffset + index;
					}
				}

				shadowVertexOffset += mesh.numVertices;
			}
		}

	}

	{
		sf::SmallStringBuf<128> name;
		name.format("MapChunk(%d,%d) vertices", chunk.chunkI.x, chunk.chunkI.y);

		sg_buffer_desc d = { };
		d.size = sizeof(sp::Vertex) * numVertices;
		d.type = SG_BUFFERTYPE_VERTEXBUFFER;
		d.usage = SG_USAGE_IMMUTABLE;
		d.content = vertices.data;
		d.label = name.data;
		chunk.mesh.vertexBuffer = sg_make_buffer(&d);
	}

	{
		sf::SmallStringBuf<128> name;
		name.format("MapChunk(%d,%d) indices", chunk.chunkI.x, chunk.chunkI.y);

		sg_buffer_desc d = { };
		d.size = (indexDst16 ? sizeof(uint16_t) : sizeof(uint32_t)) * numIndices;
		d.type = SG_BUFFERTYPE_INDEXBUFFER;
		d.usage = SG_USAGE_IMMUTABLE;
		d.content = indexDst16 ? (void*)indices16.data : (void*)indices32.data;
		d.label = name.data;
		chunk.mesh.indexBuffer = sg_make_buffer(&d);
	}

	if (shadowNumVertices > 0) {
		sf::SmallStringBuf<128> name;
		name.format("MapChunk(%d,%d) shadow vertices", chunk.chunkI.x, chunk.chunkI.y);

		sg_buffer_desc d = { };
		d.size = sizeof(sf::Vec3) * shadowNumVertices;
		d.type = SG_BUFFERTYPE_VERTEXBUFFER;
		d.usage = SG_USAGE_IMMUTABLE;
		d.content = shadowVertices.data;
		d.label = name.data;
		chunk.shadowMesh.vertexBuffer = sg_make_buffer(&d);
	}

	if (shadowNumIndices > 0) {
		sf::SmallStringBuf<128> name;
		name.format("MapChunk(%d,%d) shadow indices", chunk.chunkI.x, chunk.chunkI.y);

		sg_buffer_desc d = { };
		d.size = (shadowIndexDst16 ? sizeof(uint16_t) : sizeof(uint32_t)) * shadowNumIndices;
		d.type = SG_BUFFERTYPE_INDEXBUFFER;
		d.usage = SG_USAGE_IMMUTABLE;
		d.content = shadowIndexDst16 ? (void*)shadowIndices16.data : (void*)shadowIndices32.data;
		d.label = name.data;
		chunk.shadowMesh.indexBuffer = sg_make_buffer(&d);
	}

	chunk.mesh.numVertices = numVertices;
	chunk.mesh.numIndices = numIndices;
	chunk.mesh.bounds = sf::Bounds3::minMax(aabbMin, aabbMax);

	chunk.shadowMesh.numVertices = shadowNumVertices;
	chunk.shadowMesh.numIndices = shadowNumIndices;
	chunk.shadowMesh.bounds = sf::Bounds3::minMax(shadowAabbMin, shadowAabbMax);
	return true;
}

void MapRenderer::update()
{
	bool chunksUpdated = false;
	bool allDone = true;
	for (uint32_t i = 0; i < data->dirtyChunks.size; i++) {
		sf::Vec2i chunkI = data->dirtyChunks[i];
		MapChunk &chunk = data->chunks[chunkI];
		sf_assert(chunk.dirty);

		if (!rebuildChunk(data, chunk)) {
			allDone = false;
			continue;
		}

		data->dirtyChunks.removeSwap(i--);
		chunk.dirty = false;
		chunksUpdated = true;
	}
}

void MapRenderer::testRenderLight()
{
	float time = (float)stm_sec(stm_now()) * 0.7f;

	float lightRadius = 20.0f;

	sf::SmallArray<TestLight, 16> testLights;

	{
		float radius = 10.0f;
		testLights.push({ { cosf(time)*radius, 2.0f, sinf(time)*radius }, lightRadius, { 6.0f, 1.0f, 1.0f } });
		time += sf::F_2PI / 3.0f;
		testLights.push({ { cosf(time)*radius, 2.0f, sinf(time)*radius }, lightRadius, { 1.0f, 5.0f, 1.0f } });
		time += sf::F_2PI / 3.0f;
		testLights.push({ { cosf(time)*radius, 2.0f, sinf(time)*radius }, lightRadius, { 1.0f, 1.0f, 9.0f } });
	}


#if 0
	srand(0);
	for (size_t i = 0; i < 50; i++) {
		TestLight &light = testLights.push();
		float y = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * 16.0f;
		float x = fmodf(time*0.03f + (float)rand()/(float)RAND_MAX, 1.0f) * 128.0f - 64.0f;
		light.position = sf::Vec3(x, (float)rand()/(float)RAND_MAX*1.5f+1.0f, y);
		light.color = sf::Vec3(0.5f, 0.5f, 0.5f) * powf(sinf((float)rand()/(float)RAND_MAX * sf::F_2PI + time * 5.0f) * 0.5f + 0.5f, 2.0f);
		light.radius = 15.0f;
	}
#endif

	{
		sg_pass_action action = { };
		action.colors[0].action = SG_ACTION_LOAD;
		action.depth.action = SG_ACTION_LOAD;
		action.stencil.action = SG_ACTION_LOAD;
		sp::beginPass(data->testShadowPass, &action);
		bool topLeft = sg_query_features().origin_top_left;

		uint32_t offsetX = 0;
		uint32_t offsetY = 0;
		for (TestLight &light : testLights) {
			data->testShadowResetPipe.bind();

			sg_bindings bindings = { };
			bindings.index_buffer = data->shadowResetIndexBuffer;
			bindings.vertex_buffers[0] = data->shadowResetVertexBuffer;
			sg_apply_bindings(&bindings);

			MapShadow_Vertex_t vertexUbo;
			vertexUbo.cameraPosition = sf::Vec3(0.0f, 0.0f, 1.0f - light.radius);
			sf::Mat44 resetProj;
			resetProj.writeColMajor44(vertexUbo.worldToClip);
			sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_MapShadow_Vertex, &vertexUbo, sizeof(vertexUbo));

			uint32_t baseX = offsetX * data->testShadowExtent;
			uint32_t baseY = offsetY * data->testShadowExtent;
			sg_apply_viewport(baseX, baseY, data->testShadowExtent * 6, data->testShadowExtent, topLeft);

			light.shadowMad.x = 1.0f / data->testShadowAtlasWidth;
			light.shadowMad.y = 1.0f / data->testShadowAtlasHeight;
			light.shadowMad.z = (float)offsetX * light.shadowMad.x;
			light.shadowMad.w = (float)offsetY * light.shadowMad.y;

			sg_draw(0, 6, 1);

			sf::Vec3 cubeBasis[][2] = {
				{ { +1,0,0 }, { 0,+1,0 } },
				{ { -1,0,0 }, { 0,+1,0 } },
				{ { 0,+1,0 }, { 0,0,-1 } },
				{ { 0,-1,0 }, { 0,0,+1 } },
				{ { 0,0,+1 }, { 0,+1,0 } },
				{ { 0,0,-1 }, { 0,+1,0 } },
			};

			for (uint32_t i = 0; i < 6; i++) {
				sg_apply_viewport(baseX + i * data->testShadowExtent, baseY, data->testShadowExtent, data->testShadowExtent, topLeft);

				const sf::Vec3 *basis = cubeBasis[i];
				sf::Mat34 view = sf::mat::look(light.position, basis[0], basis[1]);
				sf::Mat44 proj = sf::mat::perspectiveD3D(sf::F_PI/2.0f, 1.0f, 0.1f, light.radius);
				sf::Mat44 worldToClip = proj * view;

				float clipNearW = sg_query_backend() == SG_BACKEND_D3D11 ? 0.0f : -1.0f;
				sf::Frustum frustum { worldToClip, clipNearW };

				vertexUbo.cameraPosition = light.position;
				worldToClip.writeColMajor44(vertexUbo.worldToClip);

				for (auto &pair : data->chunks) {
					MapChunk &chunk = pair.val;
					if (!chunk.shadowMesh.vertexBuffer.id) continue;
					if (!frustum.intersects(chunk.shadowMesh.bounds)) continue;

					data->testShadowPipe[chunk.mesh.largeIndices].bind();
					sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_MapShadow_Vertex, &vertexUbo, sizeof(vertexUbo));

					bindings.vertex_buffers[0] = data->shadowResetVertexBuffer;
					bindings.index_buffer = chunk.shadowMesh.indexBuffer;
					bindings.vertex_buffers[0] = chunk.shadowMesh.vertexBuffer;
					sg_apply_bindings(&bindings);
				
					sg_draw(0, chunk.shadowMesh.numIndices, 1);
				}

			}

			offsetX += 6;
			if (offsetX >= data->testShadowAtlasWidth) {
				offsetX = 0;
				offsetY++;
			}
		}

		sp::endPass();
	}

	{
		sg_pass_action action = { };
		action.colors[0].action = SG_ACTION_CLEAR;
		sp::beginPass(data->testLightPass, &action);

		const uint32_t BatchSize = 64;
		int pixelUboSize = sizeof(LightGrid_Pixel_t);
		for (uint32_t base = 0; base < testLights.size; base += BatchSize) {
			uint32_t num = sf::min(testLights.size - base, BatchSize);
			for (uint32_t i = 0; i < num; i++) {
				TestLight &light = testLights[base + i];
				data->testLightPixel.lightData[i*3 + 0] = sf::Vec4(light.position, light.radius);
				data->testLightPixel.lightData[i*3 + 1] = sf::Vec4(light.color, 0.0f);
				data->testLightPixel.lightData[i*3 + 2] = light.shadowMad;
			}
			data->testLightPixel.numLightsF = (float)num;

			data->testLightPipe.bind();
			sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_LightGrid_Vertex, &data->testLightVertex, sizeof(data->testLightVertex));
			sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_LightGrid_Pixel, &data->testLightPixel, pixelUboSize);

			sg_bindings bindings = { };
			bindings.fs_images[SLOT_LightGrid_shadowAtlas] = data->testShadowAtlas.image;
			bindings.vertex_buffers[0] = data->testPostVertexBuffer;
			sg_apply_bindings(&bindings);

			sg_draw(0, 3, 1);
		}

		sp::endPass();
	}
}

void MapRenderer::render()
{
	Game &game = *t_game;

	data->testPixel.cameraPosition = game.camera.position;

	float clipNearW = sg_query_backend() == SG_BACKEND_D3D11 ? 0.0f : -1.0f;
	sf::Frustum frustum { game.camera.worldToClip, clipNearW };

	sg_pipeline prevPipeline = { };
	sg_bindings bindings = { };
	for (auto &pair : data->chunks) {
		MapChunk &chunk = pair.val;
		if (!chunk.mesh.vertexBuffer.id) continue;
		if (!frustum.intersects(chunk.mesh.bounds)) continue;

		data->pipeMapTile[chunk.mesh.largeIndices].bind();

		MapTile_Vertex_t vertexUbo;
		game.camera.worldToClip.writeColMajor44(vertexUbo.worldToClip);
		sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_MapTile_Vertex, &vertexUbo, sizeof(vertexUbo));
		sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_MapTile_LightGrid, &data->testPixel, sizeof(data->testPixel));
		bindings.fs_images[SLOT_MapTile_lightGrid] = data->testLightGridImage.image;

		bindings.index_buffer = chunk.mesh.indexBuffer;
		bindings.vertex_buffers[0] = chunk.mesh.vertexBuffer;
		sg_apply_bindings(&bindings);
	
		sg_draw(0, chunk.mesh.numIndices, 1);
	}

#if 0
	{
		sgl_begin_lines();

		float view[16], proj[16];
		game.camera.worldToView.writeColMajor44(view);
		game.camera.viewToClip.writeColMajor44(proj);
		sgl_matrix_mode_modelview();
		sgl_load_matrix(view);
		sgl_matrix_mode_projection();
		sgl_load_matrix(proj);

		for (auto &pair : data->chunks) {
			MapChunk &chunk = pair.val;
			if (!chunk.rtkScene) continue;
			debugRenderBvh(chunk.rtkScene, 0);
		}
		sgl_end();
	}
#endif
}
#endif
