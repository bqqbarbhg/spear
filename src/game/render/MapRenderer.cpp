#include "MapRenderer.h"

#include "sf/Array.h"
#include "sf/HashMap.h"
#include "sp/Model.h"
#include "game/Game.h"
#include "ext/sokol/sokol_gfx.h"
#include "game/shader/MapTile.h"
#include "game/shader/LightGrid.h"
#include "ext/rtk.h"

// TEMP TEMP
#include "ext/sokol/sokol_time.h"
#include "ext/sokol/sokol_gl.h"

struct ChunkModel
{
	sp::ModelRef model;
	sf::Vec2i tile;
	sf::Mat34 transform;
	Entity entity;
	bool castShadows;
};

struct MapChunk
{
	bool dirty = false;
	sf::Vec2i chunkI;
	sf::Array<ChunkModel> models;
	sf::Vec3 aabbMin, aabbMax;

	uint32_t numVertices = 0;
	uint32_t numIndices = 0;

	sg_buffer vertexBuffer = { 0 };
	sg_buffer indexBuffer = { 0 };
	bool largeIndices = false;

	rtk_scene *rtkScene = nullptr;
};

struct MapRenderer::Data
{
	static const uint32_t ChunkSizeLog2 = 3;

	sf::HashMap<sf::Vec2i, MapChunk> chunks;
	sf::Array<sf::Vec2i> dirtyChunks;

	sg_shader shaderMapTile;
	sg_pipeline pipeMapTile[2 /* largeIndices */];

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

	MapTile_Pixel_t testPixel;
	sg_image testLightGridImage;
	sg_pass testLightPass;

	sg_shader testLightShader;
	sg_pipeline testLightPipe;
	LightGrid_Vertex_t testLightVertex;
	LightGrid_Pixel_t testLightPixel;

	sg_image testShadowImage = { 0 };
	sf::Vec3 testShadowOrigin;
	sf::Vec3 testShadowRcpScale;
	float testShadowYSlices;

	sg_buffer testPostVertexBuffer;

	rtk_scene *rtkScene = nullptr;

};

struct TestLight
{
	sf::Vec3 position;
	sf::Vec3 color;
};

MapRenderer::MapRenderer()
	: data(new Data())
{
	data->shaderMapTile = sg_make_shader(MapTile_MapTile_shader_desc());

	for (int largeIndices = 0; largeIndices < 2; largeIndices++) {
		sg_pipeline_desc d = { };
		d.shader = data->shaderMapTile;
		d.depth_stencil.depth_write_enabled = true;
		d.depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL;
		d.rasterizer.cull_mode = SG_CULLMODE_BACK;
		d.rasterizer.face_winding = SG_FACEWINDING_CCW;
		d.index_type = largeIndices ? SG_INDEXTYPE_UINT32 : SG_INDEXTYPE_UINT16;
		d.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
		d.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT3;
		d.layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT2;
		d.label = "mapTile";
		data->pipeMapTile[largeIndices] = sg_make_pipeline(&d);
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
	}

	{
		const uint32_t Extent = 64;
		const uint32_t Slices = 4;
		const uint32_t TexWidth = Extent*Slices;
		const uint32_t TexHeight = Extent*7;

		float scale = 16.0f;
		sf::Vec3 origin = sf::Vec3(-scale-0.5f, 0.0f, -scale-0.5f);
		sf::Vec3 size = sf::Vec3(scale*2.0f-1.0f, 4.0f-1.0f, scale*2.0f-1.0f);

		data->testLightVertex.lightGridYSlices = (float)Slices;
		data->testLightPixel.lightGridOrigin = origin;
		data->testLightPixel.lightGridScale = size / sf::Vec3(1.0f, (float)Slices, 1.0f);

		data->testPixel.lightGridOrigin = origin;
		data->testPixel.lightGridRcpScale = sf::Vec3(1.0f) / size;
		data->testPixel.lightGridYSlices = (float)Slices;
		data->testPixel.lightGridRcpYSlices = 1.0f / (float)Slices;

		{
			sg_image_desc d = { };
			d.width = (int)TexWidth;
			d.height = (int)TexHeight;
			d.render_target = true;
			d.pixel_format = SG_PIXELFORMAT_RGBA32F;
			d.mag_filter = SG_FILTER_LINEAR;
			d.min_filter = SG_FILTER_LINEAR;
			d.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
			d.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
			d.wrap_w = SG_WRAP_CLAMP_TO_EDGE;
			d.label = "testLightGrid";
			data->testLightGridImage = sg_make_image(&d);
		}

		{
			sg_pass_desc d = { };
			d.color_attachments[0].image = data->testLightGridImage;
			data->testLightPass = sg_make_pass(&d);
		}
	}

	{
		data->testLightShader = sg_make_shader(LightGrid_LightGrid_shader_desc());
		sg_pipeline_desc d = { };
		d.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT2;
		d.shader = data->testLightShader;
		d.blend.color_format = SG_PIXELFORMAT_RGBA32F;
		d.blend.depth_format = SG_PIXELFORMAT_NONE;
		d.blend.enabled = true;
		d.blend.src_factor_rgb = d.blend.src_factor_alpha = SG_BLENDFACTOR_ONE;
		d.blend.dst_factor_rgb = d.blend.dst_factor_alpha = SG_BLENDFACTOR_ONE;
		d.label = "testLight";
		data->testLightPipe = sg_make_pipeline(&d);
	}

#if 0
	if (false)
	{
		const uint32_t Extent = 32;
		const uint32_t Slices = 8;
		const uint32_t TexWidth = Extent*Slices;
		const uint32_t TexHeight = Extent*6;
		sf::Array<uint16_t> dataArr;
		dataArr.resizeUninit(TexWidth*TexHeight*4);
		uint16_t *dataPtr = dataArr.data;

		float scale = 32.0f;
		sf::Vec3 origin = sf::Vec3(-scale, -4.0f, -scale);
		sf::Vec3 size = sf::Vec3(scale*2.0f, 12.0f, scale*2.0f);

		sf::Vec3 normals[6] = {
			{ 1,0,0 }, { -1,0,0 }, { 0,1,0 }, { 0,-1,0 }, { 0,0,1 }, { 0,0,-1 },
		};

		for (uint32_t d = 0; d < 6; d++)
		for (uint32_t z = 0; z < Extent; z++)
		for (uint32_t y = 0; y < Slices; y++)
		for (uint32_t x = 0; x < Extent; x++)
		{
			sf::Vec3 p = sf::Vec3((float)x, (float)y, (float)z) / sf::Vec3(Extent, Slices, Extent) * size + origin;
			sf::Vec3 n = normals[d];

			sf::Vec3 sum;
			for (TestLight &light : testLights) {
				sf::Vec3 delta = light.position - p;
				sf::Vec3 l = sf::normalize(delta);
				float attenuation = 1.0f / (1.0f + sf::lengthSq(delta));
				float r = sf::dot(l, n) * attenuation;
				sum += light.color * r;
			}

			sf::Vec4 color = sf::Vec4(sum.x, sum.y, sum.z, 1.0f);

			uint32_t u = x + y * Extent;
			uint32_t v = z + d * Extent;
			uint32_t base = (v * TexWidth + u) * 4;
			dataPtr[base + 0] = (uint16_t)sf::clamp(color.x * 65535.9f, 0.0f, 65535.0f);
			dataPtr[base + 1] = (uint16_t)sf::clamp(color.y * 65535.9f, 0.0f, 65535.0f);
			dataPtr[base + 2] = (uint16_t)sf::clamp(color.z * 65535.9f, 0.0f, 65535.0f);
			dataPtr[base + 3] = (uint16_t)sf::clamp(color.w * 65535.9f, 0.0f, 65535.0f);
		}

		sg_image_desc d = { };
		d.width = (int)TexWidth;
		d.height = (int)TexHeight;
		d.pixel_format = SG_PIXELFORMAT_RGBA16;
		d.mag_filter = SG_FILTER_LINEAR;
		d.min_filter = SG_FILTER_LINEAR;
		d.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
		d.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
		d.wrap_w = SG_WRAP_CLAMP_TO_EDGE;
		d.content.subimage[0][0].ptr = dataArr.data;
		d.content.subimage[0][0].size = (int)dataArr.byteSize();
		d.label = "testLightGrid";
		data->testLightGridImage = sg_make_image(&d);

		data->testPixel.lightGridOrigin = origin;
		data->testPixel.lightGridRcpScale = sf::Vec3(1.0f) / size;
		data->testPixel.lightGridYSlices = (float)Slices;
		data->testPixel.lightGridRcpYSlices = 1.0f / (float)Slices;
	}
#endif
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
	chunkModel.tile = tile;
	chunkModel.transform = model.transform;
	chunkModel.castShadows = model.castShadows;
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
	uint32_t shadowVertices = 0;
	uint32_t shadowIndices = 0;

	for (ChunkModel &model : chunk.models) {
		if (!model.model.isLoaded()) return false;

		for (sp::Mesh &mesh : model.model->meshes) {
			numVertices += mesh.numVertices;
			numIndices += mesh.numIndices;
			if (model.castShadows) {
				shadowVertices += mesh.numVertices;
				shadowIndices += mesh.numIndices;
			}
		}
	}

	sg_destroy_buffer(chunk.vertexBuffer);
	sg_destroy_buffer(chunk.indexBuffer);
	rtk_free_scene(chunk.rtkScene);

	sf::Array<sp::Vertex> vertices;
	sf::Array<uint16_t> indices16;
	sf::Array<uint32_t> indices32;
	uint16_t *indexDst16 = NULL;
	uint32_t *indexDst32 = NULL;
	vertices.resizeUninit(numVertices);
	if (numVertices > UINT16_MAX) {
		indices32.resizeUninit(numIndices);
		indexDst32 = indices32.data;
		chunk.largeIndices = true;
	} else {
		indices16.resizeUninit(numIndices);
		indexDst16 = indices16.data;
		chunk.largeIndices = false;
	}
	sp::Vertex *vertexDst = vertices.data;

	sf::Vec3 aabbMin = sf::Vec3(+HUGE_VALF);
	sf::Vec3 aabbMax = sf::Vec3(-HUGE_VALF);

	sf::Array<rtk_mesh> rtkMeshes;
	sf::Array<uint32_t> rtkIndices;
	rtkIndices.resizeUninit(shadowIndices);
	uint32_t *rtkIndexDst = rtkIndices.data;

	uint32_t vertexOffset = 0;
	for (ChunkModel &model : chunk.models) {
		for (sp::Mesh &mesh : model.model->meshes) {
			if (model.castShadows) {
				rtk_mesh &rtkMesh = rtkMeshes.push();
				rtkMesh.vertices = (rtk_vec3*)vertexDst;
				rtkMesh.vertices_stride = sizeof(sp::Vertex);
				rtkMesh.indices = rtkIndexDst;
				rtkMesh.num_triangles = mesh.numIndices / 3;
				rtkMesh.transform = rtk_identity;

				for (uint16_t index : mesh.indexData) {
					*rtkIndexDst++ = index;
				}
			}

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
	}

	if (rtkMeshes.size) {
		rtk_scene_desc rtkScene = { };
		rtkScene.meshes = rtkMeshes.data;
		rtkScene.num_meshes = rtkMeshes.size;
		chunk.rtkScene = rtk_create_scene(&rtkScene);
	} else {
		chunk.rtkScene = nullptr;
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
		chunk.vertexBuffer = sg_make_buffer(&d);
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
		chunk.indexBuffer = sg_make_buffer(&d);
	}

	chunk.numVertices = numVertices;
	chunk.numIndices = numIndices;
	return true;
}

static rtk_vec3 toRtk(const sf::Vec3 &v) { rtk_vec3 r = { v.x, v.y, v.z }; return r; }

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

	if (chunksUpdated && allDone) {
		rtk_free_scene(data->rtkScene);

		sf::Array<rtk_primitive> rtkPrims;
		rtkPrims.reserve(data->chunks.size());
		for (auto pair : data->chunks) {
			MapChunk &chunk = pair.val;
			if (!chunk.rtkScene) continue;

			rtk_primitive &rtkPrim = rtkPrims.push();
			rtk_init_subscene(&rtkPrim, chunk.rtkScene, nullptr);
		}

		rtk_scene_desc rtkScene = { };
		rtkScene.primitives = rtkPrims.data;
		rtkScene.num_primitives = rtkPrims.size;
		data->rtkScene = rtk_create_scene(&rtkScene);

		sg_destroy_image(data->testShadowImage);

		{
			const uint32_t Extent = 64;
			const uint32_t Slices = 4;
			const uint32_t TexWidth = Extent*Slices;
			const uint32_t TexHeight = Extent;

			float scale = 16.0f;
			sf::Vec3 origin = sf::Vec3(-scale-0.5f, 0.0f, -scale-0.5f);
			sf::Vec3 size = sf::Vec3(scale*2.0f-1.0f, 4.0f-1.0f, scale*2.0f-1.0f);

			data->testShadowOrigin = origin;
			data->testShadowRcpScale = sf::Vec3(1.0f) / size;
			data->testShadowYSlices = (float)Slices;

			sf::Array<uint8_t> dataArr;
			dataArr.resizeUninit(TexWidth*TexHeight);
			uint8_t *dataPtr = dataArr.data;

			sf::Vec3 lightPos { 0.0f, 0.8f, 0.0f };

			for (uint32_t z = 0; z < Extent; z++)
			for (uint32_t y = 0; y < Slices; y++)
			for (uint32_t x = 0; x < Extent; x++)
			{
				sf::Vec3 p = sf::Vec3((float)x, (float)y, (float)z) / sf::Vec3(Extent-1, Slices-1, Extent-1) * size + origin;

				uint32_t occluded = 0, total = 0;

				srand(0);

				sf::Vec3 step = sf::Vec3(1.5f) / sf::Vec3(Extent, Slices, Extent) * size;
				for (uint32_t i = 0; i < 16; i++) {
					sf::Vec3 dp = p;

					sf::Vec3 d1;
					if (i > 0) {
						d1.x = (float)rand()/(float)RAND_MAX * 2.0f - 1.0f;
						d1.y = (float)rand()/(float)RAND_MAX * 2.0f - 1.0f;
						d1.z = (float)rand()/(float)RAND_MAX * 2.0f - 1.0f;
					}

					sf::Vec3 lp = lightPos + d1 * 0.7f;
					sf::Vec3 dir = lp - dp;
					float dist = sf::length(dir);

					rtk_ray ray;
					ray.origin = toRtk(dp);
					ray.direction = toRtk(dir / dist);
					ray.min_t = 0.5f;
					rtk_hit hit;
					if (rtk_raytrace(data->rtkScene, &ray, &hit, dist)) {
						occluded++;
					} else {
						occluded = occluded;
					}
					total++;
				}

				uint32_t u = x + y * Extent;
				uint32_t v = z;
				uint32_t base = v * TexWidth + u;
				float shadow = 1.0f - ((float)occluded / (float)total);
				dataPtr[base + 0] = (uint8_t)sf::clamp(shadow * 255.9f, 0.0f, 255.0f);
				// dataPtr[base + 0] = (p.x >= -3.1f && p.x <= 3.1f) ? 255 : 0;
			}

			sg_image_desc d = { };
			d.width = (int)TexWidth;
			d.height = (int)TexHeight;
			d.pixel_format = SG_PIXELFORMAT_R8;
			d.mag_filter = SG_FILTER_LINEAR;
			d.min_filter = SG_FILTER_LINEAR;
			d.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
			d.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
			d.wrap_w = SG_WRAP_CLAMP_TO_EDGE;
			d.content.subimage[0][0].ptr = dataArr.data;
			d.content.subimage[0][0].size = (int)dataArr.byteSize();
			d.label = "testShadow";
			data->testShadowImage = sg_make_image(&d);

		}
	}
}

void MapRenderer::testRenderLight()
{
	float time = (float)stm_sec(stm_now());
	float radius = 10.0f;

	sf::SmallArray<TestLight, 16> testLights;
	testLights.push({ { 0.0f, 3.0f, 0.0f }, { 10.0f, 5.0f, 3.0f } });
	time += sf::F_2PI / 3.0f;
	// testLights.push({ { cosf(time)*radius, 3.0f, sinf(time)*radius }, { 1.0f, 8.0f, 1.0f } });
	time += sf::F_2PI / 3.0f;
	// testLights.push({ { cosf(time)*radius, 3.0f, sinf(time)*radius }, { 1.0f, 1.0f, 15.0f } });

#if 0
	srand(0);
	for (size_t i = 0; i < 250; i++) {
		TestLight &light = testLights.push();
		float y = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * 16.0f;
		float x = fmodf(time*0.03f + (float)rand()/(float)RAND_MAX, 1.0f) * 128.0f - 64.0f;
		light.position = sf::Vec3(x, (float)rand()/(float)RAND_MAX*1.0f+1.0f, y);
		light.color = sf::Vec3(0.2f, 0.2f, 0.2f) * powf(sinf((float)rand()/(float)RAND_MAX * sf::F_2PI + time * 5.0f) * 0.5f + 0.5f, 2.0f);
	}
#endif

	sg_pass_action action = { };
	action.colors[0].action = SG_ACTION_CLEAR;
	sg_begin_pass(data->testLightPass, &action);

	if (data->testShadowImage.id) {
		const uint32_t BatchSize = 64;
		for (uint32_t base = 0; base < testLights.size; base += BatchSize) {
			uint32_t num = sf::min(testLights.size - base, BatchSize);
			for (uint32_t i = 0; i < num; i++) {
				TestLight &light = testLights[base + i];
				data->testLightPixel.lightData[i*4 + 0] = sf::Vec4(light.position, 0.0f);
				data->testLightPixel.lightData[i*4 + 1] = sf::Vec4(light.color, 0.0f);
				data->testLightPixel.lightData[i*4 + 2] = sf::Vec4(data->testShadowOrigin, data->testShadowYSlices);
				data->testLightPixel.lightData[i*4 + 3] = sf::Vec4(data->testShadowRcpScale, 1.0f / data->testShadowYSlices);
			}
			data->testLightPixel.numLightsF = (float)num;

			sg_apply_pipeline(data->testLightPipe);
			sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_LightGrid_Vertex, &data->testLightVertex, sizeof(data->testLightVertex));
			sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_LightGrid_Pixel, &data->testLightPixel, sizeof(data->testLightPixel));

			sg_bindings bindings = { };
			bindings.fs_images[SLOT_LightGrid_shadowGrid] = data->testShadowImage;
			bindings.vertex_buffers[0] = data->testPostVertexBuffer;
			sg_apply_bindings(&bindings);

			sg_draw(0, 3, 1);
		}
	}

	sg_end_pass();
}

static void debugRenderBvh(rtk_scene *scene, uintptr_t parent)
{
	rtk_bvh bvh = rtk_get_bvh(scene, parent);

	rtk_vec3 a = bvh.bounds.min, b = bvh.bounds.max;

	sgl_c3f(1.0f, 0.0f, 0.0f);
	sgl_v3f(a.x, a.y, a.z); sgl_v3f(b.x, a.y, a.z);
	sgl_v3f(a.x, b.y, a.z); sgl_v3f(b.x, b.y, a.z);
	sgl_v3f(a.x, a.y, b.z); sgl_v3f(b.x, a.y, b.z);
	sgl_v3f(a.x, b.y, b.z); sgl_v3f(b.x, b.y, b.z);
	sgl_v3f(a.x, a.y, a.z); sgl_v3f(a.x, a.y, b.z);
	sgl_v3f(a.x, a.y, b.z); sgl_v3f(a.x, b.y, b.z);
	sgl_v3f(a.x, b.y, b.z); sgl_v3f(a.x, b.y, a.z);
	sgl_v3f(a.x, b.y, a.z); sgl_v3f(a.x, a.y, a.z);
	sgl_v3f(b.x, a.y, a.z); sgl_v3f(b.x, a.y, b.z);
	sgl_v3f(b.x, a.y, b.z); sgl_v3f(b.x, b.y, b.z);
	sgl_v3f(b.x, b.y, b.z); sgl_v3f(b.x, b.y, a.z);
	sgl_v3f(b.x, b.y, a.z); sgl_v3f(b.x, a.y, a.z);

	if (bvh.leaf == 0) {
		for (size_t i = 0; i < 4; i++) {
			debugRenderBvh(scene, bvh.child[i]);
		}
	} else {
		rtk_leaf leaf;
		rtk_get_leaf(scene, bvh.leaf, &leaf);

#if 0
		sgl_c3f(0.3f, 0.3f, 1.0f);
		for (uint32_t i = 0; i < leaf.num_triangles; i++) {
			rtk_leaf_triangle &tri = leaf.triangles[i];
			for (uint32_t j = 0; j < 3; j++) {
				sgl_v3f(tri.v[j].x, tri.v[j].y, tri.v[j].z);
				sgl_v3f(tri.v[(j+1)%3].x, tri.v[(j+1)%3].y, tri.v[(j+1)%3].z);
			}
		}
#endif
	}
}

void MapRenderer::render()
{
	Game &game = *t_game;

	data->testPixel.cameraPosition = game.camera.position;

	sg_pipeline prevPipeline = { };
	sg_bindings bindings = { };
	for (auto &pair : data->chunks) {
		MapChunk &chunk = pair.val;
		if (!chunk.vertexBuffer.id) continue;
		// TODO: Frustum culling

		sg_pipeline pipe = data->pipeMapTile[chunk.largeIndices];
		if (pipe.id != prevPipeline.id) {
			sg_apply_pipeline(pipe);
		}

		MapTile_Vertex_t vertexUbo;
		game.camera.worldToClip.writeColMajor44(vertexUbo.worldToClip);
		sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_MapTile_Vertex, &vertexUbo, sizeof(vertexUbo));

		sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_MapTile_Pixel, &data->testPixel, sizeof(data->testPixel));

		bindings.fs_images[SLOT_MapTile_lightGrid] = data->testLightGridImage;
		bindings.index_buffer = chunk.indexBuffer;
		bindings.vertex_buffers[0] = chunk.vertexBuffer;
		sg_apply_bindings(&bindings);
		
		sg_draw(0, chunk.numIndices, 1);
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
