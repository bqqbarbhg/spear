#include "MapRenderer.h"

#include "sf/Array.h"
#include "sf/HashMap.h"
#include "sp/Model.h"
#include "game/Game.h"
#include "ext/sokol/sokol_gfx.h"
#include "game/shader/MapTile.h"

struct ChunkModel
{
	sp::ModelRef model;
	sf::Vec2i tile;
	sf::Mat34 transform;
	Entity entity;
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

	for (ChunkModel &model : chunk.models) {
		if (!model.model.isLoaded()) return false;

		for (sp::Mesh &mesh : model.model->meshes) {
			numVertices += mesh.numVertices;
			numIndices += mesh.numIndices;
		}
	}

	sg_destroy_buffer(chunk.vertexBuffer);
	sg_destroy_buffer(chunk.indexBuffer);

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

	uint32_t vertexOffset = 0;
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

void MapRenderer::update()
{
	for (uint32_t i = 0; i < data->dirtyChunks.size; i++) {
		sf::Vec2i chunkI = data->dirtyChunks[i];
		MapChunk &chunk = data->chunks[chunkI];
		sf_assert(chunk.dirty);

		if (!rebuildChunk(data, chunk)) continue;

		data->dirtyChunks.removeSwap(i--);
		chunk.dirty = false;
	}
}

void MapRenderer::render()
{
	Game &game = *t_game;
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

		bindings.index_buffer = chunk.indexBuffer;
		bindings.vertex_buffers[0] = chunk.vertexBuffer;
		sg_apply_bindings(&bindings);
		
		sg_draw(0, chunk.numIndices, 1);
	}
}
