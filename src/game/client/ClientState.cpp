#include "ClientState.h"

#include "sf/Random.h"

#include "game/shader/GameShaders.h"
#include "game/shader/MapShadow.h"

namespace cl {

static sf::Box<Entity> convertEntity(const sf::Box<sv::Entity> &svEntity)
{
	sf::Box<Entity> data;
	if (auto ent = svEntity->as<sv::Character>()) {
		auto chr = sf::box<Character>(svEntity);
		data = chr;

		chr->model.load(ent->model);

	} else {
		sf_failf("Unhandled entity type: %u", svEntity->type);
	}

	data->position = sf::Vec2(svEntity->position);

	return data;
}

static bool generateMapMesh(sf::Array<cl::MapMesh> &meshes, sf::Random &tileRng, const TileInfoRef &tileRef, const sf::Vec2i &tilePos)
{
	if (!tileRef.isLoaded()) {
		return !tileRef.isLoading();
	}

	TileInfo &info = tileRef->data;
	TileVariantInfo &variant = info.getVariant(tileRng.nextFloat());

	if (variant.modelRef.isLoading()) return false;
	if (variant.shadowModelRef.isLoading()) return false;

	float rotation = (float)(tileRng.nextU32() & 3) * (sf::F_2PI * 0.25f);
	MapMesh &mesh = meshes.push();
	if (variant.modelRef) mesh.model = variant.modelRef;
	if (variant.shadowModelRef) mesh.shadowModel = variant.shadowModelRef;

	float scale = variant.scale * info.scale;

	sf::Vec3 pos = { (float)tilePos.x, 0.0f, (float)tilePos.y };

	mesh.transform = sf::mat::translate(pos) * sf::mat::rotateY(rotation) * sf::mat::scale(scale);

	return true;
}

static bool generateMapMeshes(sf::Array<cl::MapMesh> &meshes, cl::State &state, sv::Map &svMap, const sf::Vec2i &chunkPos)
{
	meshes.clear();

	uint32_t hash = sf::hash(chunkPos);
	sf::Random chunkRng { hash };

	auto chunkIt = svMap.chunks.find(chunkPos);
	if (!chunkIt) return true;
	sv::MapChunk &svChunk = chunkIt->val;

	sf::Vec2i origin = chunkPos * (int32_t)sv::MapChunk::Size;

	for (uint32_t y = 0; y < sv::MapChunk::Size; y++)
	for (uint32_t x = 0; x < sv::MapChunk::Size; x++)
	{
		sf::Random tileRng { chunkRng.nextU32(), y * sv::MapChunk::Size + x };
		tileRng.nextU32();

		sv::TileId tileId = svChunk.tiles[y * sv::MapChunk::Size + x];
		TileType &tileType = state.tileTypes[tileId];
		sf::Vec2i tilePos = sf::Vec2i((int32_t)x, (int32_t)y) + origin;

		if (!generateMapMesh(meshes, tileRng, tileType.floor, tilePos)) return false;
		if (!generateMapMesh(meshes, tileRng, tileType.tile, tilePos)) return false;
	}

	return true;
}

void State::reset(sv::State *svState)
{
	entities.clear();
	entities.resize(svState->entities.size);

	{
		uint32_t ix = 0;
		for (sf::Box<sv::Entity> &svEntity : svState->entities) {
			if (svEntity) {
				entities[ix] = convertEntity(svEntity);
			}
			ix++;
		}
	}

	tileTypes.clear();
	tileTypes.reserve(svState->map.tileTypes.size);

	{
		for (sv::TileType &tileType : svState->map.tileTypes) {
			cl::TileType &dst = tileTypes.push();
			if (tileType.floorName) dst.floor.load(tileType.floorName);
			if (tileType.tileName) dst.tile.load(tileType.tileName);
		}
	}

	chunks.clear();
	dirtyChunks.clear();
	chunks.reserve(svState->map.chunks.size());
	dirtyChunks.reserve(svState->map.chunks.size());

	for (auto &pair : svState->map.chunks) {
		chunks[pair.key].dirty = true;
		dirtyChunks.push(pair.key);
	}
}

void State::applyEvent(sv::Event *event)
{
	if (auto e = event->as<sv::EventMove>()) {

		Entity *data = entities[e->entity];

		if (auto d = data->as<Character>()) {

			if (e->waypoints.size) {
				d->waypoints.push(e->waypoints);
			} else {
				d->waypoints.clear();
				data->position = sf::Vec2(e->position);
			}

		} else {
			data->position = sf::Vec2(e->position);
		}

	} else if (auto e = event->as<sv::EventSpawn>()) {

		sv::EntityId id = e->data->id;
		sf_assert(id != 0);
		while (id >= entities.size) entities.push();
		sf_assert(!entities[id]);
		sf::Box<Entity> entity = convertEntity(e->data);
		entity->position = sf::Vec2(e->data->position);
		entities[id] = entity;

	} else if (auto e = event->as<sv::EventDestroy>()) {

		entities[e->entity].reset();

	} else {
		sf_failf("Unhandled event type: %u", e->type);
	}
}

void State::renderShadows(const RenderShadowArgs &args)
{
	// Map chunks
	{
		sg_bindings bindings = { };
		bool first = true;
		for (auto &pair : chunks) {
			MapChunk &chunk = pair.val;
			MapGeometry &geo = chunk.geometry.shadow;
			if (!geo.indexBuffer.buffer.id) continue;
			if (!args.frustum.intersects(geo.bounds)) continue;

			sp::Pipeline &pipe = gameShaders.mapChunkShadowPipe[geo.largeIndices];
			if (pipe.bind() || first) {
				first = false;
				MapShadow_Vertex_t vu;
				vu.cameraPosition = args.cameraPosition;
				args.worldToClip.writeColMajor44(vu.worldToClip);
				sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_MapShadow_Vertex, &vu, sizeof(vu));
			}

			bindings.vertex_buffers[0] = geo.vertexBuffer.buffer;
			bindings.index_buffer = geo.indexBuffer.buffer;
			sg_apply_bindings(&bindings);

			sg_draw(0, geo.numInidces, 1);
		}
	}
}

void State::updateMapChunks(sv::State &svState)
{

	for (uint32_t i = 0; i < dirtyChunks.size; i++) {
		sf::Vec2i chunkPos = dirtyChunks[i];
		MapChunk &chunk = chunks[chunkPos];
		sf_assert(chunk.dirty);

		if (!generateMapMeshes(chunk.meshes, *this, svState.map, chunkPos)) {
			continue;
		}

		chunk.geometry.build(chunk.meshes, chunkPos);

		dirtyChunks.removeSwap(i--);
		chunk.dirty = false;
	}
}

void State::recreateTargets()
{
	shadowCache.recreateTargets();
}

}
