#include "ClientState.h"

#include "sf/Random.h"

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
		float rotation = (float)(tileRng.nextU32() & 3) * (sf::F_2PI * 0.25f);

		sv::TileId tileId = svChunk.tiles[y * sv::MapChunk::Size + x];
		TileType &tileType = state.tileTypes[tileId];
		if (tileType.tile.isLoading()) return false;
		if (!tileType.tile.isLoaded()) continue;

		TileInfo &info = tileType.tile->data;
		TileVariantInfo &variant = info.getVariant(tileRng.nextFloat());

		if (variant.modelRef.isLoading()) return false;
		if (variant.shadowModelRef.isLoading()) return false;

		MapMesh &mesh = meshes.push();
		if (variant.modelRef) mesh.model = variant.modelRef;
		if (variant.shadowModelRef) mesh.model = variant.shadowModelRef;

		float scale = variant.scale * info.scale;

		sf::Vec2i tilePos = sf::Vec2i((int32_t)x, (int32_t)y) + origin;
		sf::Vec3 pos = { (float)tilePos.x, 0.0f, (float)tilePos.y };

		mesh.transform = sf::mat::translate(pos) * sf::mat::rotateY(rotation) * sf::mat::scale(scale);
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
			if (tileType.name) {
				dst.tile.load(tileType.name);
			}
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

}
