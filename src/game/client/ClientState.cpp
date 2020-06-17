#include "ClientState.h"

#include "sf/Random.h"

#include "game/shader/GameShaders.h"
#include "game/shader/MapShadow.h"

namespace cl {

static const sf::Symbol g_defaultTileMaterial { "Assets/Tiles/Default_Material/TileDefault" };

static Card convertCard(const sv::Card &svCard)
{
	Card card;

	card.svCard = svCard;
	card.imageSprite.load(svCard.type->image);

	return card;
}

static sf::Box<Entity> convertEntity(const sf::Box<sv::Entity> &svEntity)
{
	sf::Box<Entity> data;
	if (auto ent = svEntity->as<sv::Character>()) {
		auto chr = sf::box<Character>(svEntity);
		data = chr;

		chr->model.load(ent->model);
		chr->cards.reserve(ent->cards.size);
		for (sv::Card &svCard : ent->cards) {
			chr->cards.push(convertCard(svCard));
		}

	} else {
		sf_failf("Unhandled entity type: %u", svEntity->type);
	}

	data->position = sf::Vec2(svEntity->position);

	return data;
}

static void convertObjectType(ObjectType &type, const sv::GameObject &svType)
{
	type.svType = svType;
	type.mapMeshes.clear();
	type.pointLights.clear();

	for (sv::Component *component : svType.components) {
		if (sv::ModelComponent *c = component->as<sv::ModelComponent>()) {

			MapMesh &mesh = type.mapMeshes.push();
			mesh.material.load(c->material);

			sp::ModelProps props;
			props.cpuData = true;
			if (c->model) mesh.model.load(c->model, props);
			if (c->material) {
				mesh.material.load(c->material);
			} else {
				mesh.material.load(g_defaultTileMaterial);
			}

			mesh.transform =
				sf::mat::translate(c->position) * (
				sf::mat::rotateZ(c->rotation.z * (sf::F_PI/180.0f)) *
				sf::mat::rotateY(c->rotation.y * (sf::F_PI/180.0f)) *
				sf::mat::rotateX(c->rotation.x * (sf::F_PI/180.0f)) *
				sf::mat::scale(c->stretch * c->scale));

			if (c->castShadows) {
				if (c->shadowModel) {
					mesh.shadowModel.load(c->shadowModel, props);
				} else {
					mesh.shadowModel = mesh.model;
				}
			}
		} else if (sv::PointLightComponent *c = component->as<sv::PointLightComponent>()) {
			PointLight &light = type.pointLights.push();
			light.color = c->color * c->intensity;
			light.radius = c->radius;
			light.position = c->position;
			light.shadowIndex = 0;
		}
	}
}

static bool generateMapMesh(sf::Array<cl::MapMesh> &meshes, sf::Random &tileRng, const TileInfoRef &tileRef, const sf::Vec2i &tilePos)
{
	if (!tileRef.isLoaded()) {
		return !tileRef.isLoading();
	}

	TileInfo &info = tileRef->data;
	TileVariantInfo &variant = info.getVariant(tileRng.nextFloat());

	float rotation = (float)(tileRng.nextU32() & 3) * (sf::F_2PI * 0.25f);
	MapMesh &mesh = meshes.push();
	if (variant.modelRef) mesh.model = variant.modelRef;
	if (variant.shadowModelRef) mesh.shadowModel = variant.shadowModelRef;
	mesh.material = variant.materialRef;

	float scale = variant.scale * info.scale;

	sf::Vec3 pos = { (float)tilePos.x, 0.0f, (float)tilePos.y };

	mesh.transform = sf::mat::translate(pos) * sf::mat::rotateY(rotation) * sf::mat::scale(scale);

	return true;
}

static bool generateMapMeshes(sf::Array<cl::MapMesh> &meshes, cl::State &state, sv::Map &svMap, MapChunk &chunk, const sf::Vec2i &chunkPos)
{
	meshes.clear();

	uint32_t hash = sf::hash(chunkPos);
	sf::Random chunkRng { hash };

	sf::Vec2i origin = chunkPos * (int32_t)sv::MapChunk::Size;

	for (uint32_t id : chunk.meshObjects) {
		Object &object = state.objects[id];
		ObjectType &type = state.objectTypes[object.svObject.type];

		sf::Mat34 transform = state.getObjectTransform(object.svObject);
		for (MapMesh &mesh : type.mapMeshes) {
			MapMesh &dst = meshes.push();
			dst = mesh;
			dst.transform = transform * mesh.transform;
		}
	}

	return true;
}

static void setTileType(TileType &type, sv::TileType svType)
{
	if (svType.floorName) type.floor.load(svType.floorName);
	if (svType.tileName) type.tile.load(svType.tileName);
}

sf::Sphere State::getObjectTypeBounds(ObjectType &type)
{
	if (type.hasValidBounds) return type.bounds;

	sf::Sphere bounds = { { }, -1.0f };
	float lightRadius = sf::length(sf::Vec3(0.1f));

	bool loading = false;
	for (MapMesh &mapMesh : type.mapMeshes) {
		loading |= mapMesh.model.isLoading();
		loading |= mapMesh.shadowModel.isLoading();

		if (mapMesh.model.isLoaded()) {
			for (const sp::Mesh &mesh : mapMesh.model->meshes) {
				sf::Sphere sphere = sf::sphereFromBounds3(mesh.bounds, mapMesh.transform);
				bounds = sf::sphereUnion(bounds, sphere);
			}
		}
		if (mapMesh.shadowModel.isLoaded()) {
			for (const sp::Mesh &mesh : mapMesh.shadowModel->meshes) {
				sf::Sphere sphere = sf::sphereFromBounds3(mesh.bounds, mapMesh.transform);
				bounds = sf::sphereUnion(bounds, sphere);
			}
		}
	}

	for (PointLight &light : type.pointLights) {
		sf::Sphere sphere = { light.position, lightRadius };
		bounds = sf::sphereUnion(bounds, sphere);
	}

	if (!loading) {
		type.bounds = bounds;
		type.hasValidBounds = true;
	}

	return bounds;
}

sf::Vec3 State::getObjectPosition(const sv::Object &object)
{
	// TODO: Apply offset
	sf::Vec3 pos = { (float)object.x, 0.0f, (float)object.y };
	return pos;
}

sf::Mat34 State::getObjectTransform(const sv::Object &object)
{
	sf::Vec3 pos = { (float)object.x, 0.0f, (float)object.y };
	return sf::mat::translate(pos) * sf::mat::rotateY((float)object.rotation * (sf::F_2PI / 256.0f));
}

void State::getObjectBounds(const ObjectType &type, const sf::Mat34 &transform, sf::Array<sf::Mat34> &bounds)
{
	float lightRadius = 0.1f;
	for (const MapMesh &mapMesh : type.mapMeshes) {
		sf::Mat34 meshTransform = transform * mapMesh.transform;
		if (mapMesh.model.isLoaded()) {
			for (const sp::Mesh &mesh : mapMesh.model->meshes) {
				bounds.push(meshTransform * sf::obbFromBounds3(mesh.bounds));
			}
		}
		if (mapMesh.shadowModel.isLoaded()) {
			for (const sp::Mesh &mesh : mapMesh.shadowModel->meshes) {
				bounds.push(meshTransform * sf::obbFromBounds3(mesh.bounds));
			}
		}
	}

	for (const PointLight &light : type.pointLights) {
		sf::Vec3 origin = sf::transformPoint(transform, light.position);
		bounds.push(sf::mat::translate(origin) * sf::mat::scale(lightRadius));
	}
}

uint32_t State::pickObject(float &outT, const sf::Ray &ray)
{
	uint32_t minId = 0;
	float minT = HUGE_VALF;

	for (ObjectType &type : objectTypes) {
		sf::Sphere bounds = getObjectTypeBounds(type);

		for (uint32_t id : type.objects) {
			Object &object = objects[id];
			sf::Mat34 transform = getObjectTransform(object.svObject);

			// TODO: Rotation
			sf::Sphere objectBounds = { sf::transformPoint(transform, bounds.origin), bounds.radius };

			float t;
			if (sf::intesersectRay(t, ray, objectBounds) && t >= 0.0f) {
				if (t < minT) {
					sf::SmallArray<sf::Mat34, 32> obbs;
					getObjectBounds(type, transform, obbs);

					float realT = HUGE_VALF;
					for (sf::Mat34 &obb : obbs) {
						if (sf::intesersectRayObb(t, ray, obb) && t >= 0.0f) {
							realT = sf::min(realT, t);
						}
					}

					if (realT < minT) {
						minT = realT;
						minId = id;
					}
				}
			}
		}
	}

	outT = minT;
	return minId;
}

static void updateObjectImp(State &state, uint32_t id, const ObjectType &prevType, const Object &prev, const sv::Object &next)
{
	bool added = (prev.svObject.type == 0);
	bool removed = (next.type == 0);
	ObjectType &nextType = state.objectTypes[next.type];
	sf::Vec2i tile = sf::Vec2i(next.x, next.y);
	sf::Vec2i chunkI = sv::Map::getChunk(tile);

	if (!added) {
		sf::Vec2i oldTile = sf::Vec2i(prev.svObject.x, prev.svObject.y);
		sf::Vec2i oldChunkI = sv::Map::getChunk(oldTile);
		if (prevType.mapMeshes.size > 0 && (chunkI != oldChunkI || removed)) {
			MapChunk &chunk = state.chunks[oldChunkI];
			chunk.meshesDirty = true;
			chunk.meshObjects.remove(id);
			if (!chunk.dirty) {
				chunk.dirty = true;
				state.dirtyChunks.push(oldChunkI);
			}
		}
		if (prevType.pointLights.size > 0 && removed) {
			sf::Array<uint32_t> &lightIndices = state.pointLightMapping[id];
			for (uint32_t index : lightIndices) {
				state.pointLights.removeSwap(index);
				if (index < state.pointLights.size) {
					PointLight &swapLight = state.pointLights[index];
					sf::Array<uint32_t> &swapIndices = state.pointLightMapping[swapLight.objectId];
					for (uint32_t &swapRef : swapIndices) {
						if (swapRef == state.pointLights.size) {
							swapRef = index;
							break;
						}
					}
				}
			}
			state.pointLightMapping.remove(id);
		}
	}

	if (nextType.mapMeshes.size > 0) {
		MapChunk &chunk = state.chunks[chunkI];
		chunk.meshesDirty = true;
		chunk.meshObjects.insert(id);
		if (!chunk.dirty) {
			chunk.dirty = true;
			state.dirtyChunks.push(chunkI);
		}
	}

	if (nextType.pointLights.size > 0) {
		sf::Mat34 transform = state.getObjectTransform(next);
		sf::Array<uint32_t> &lightIndices = state.pointLightMapping[id];
		lightIndices.reserve(nextType.pointLights.size);
		while (lightIndices.size < nextType.pointLights.size) lightIndices.push(~0u);
		uint32_t *pLightIndex = lightIndices.data;
		for (const PointLight &src : nextType.pointLights) {
			if (*pLightIndex == ~0u) {
				*pLightIndex = state.pointLights.size;
				state.pointLights.push();
			}

			PointLight &dst = state.pointLights[*pLightIndex];
			dst.position = sf::transformPoint(transform, src.position);
			dst.color = src.color;
			dst.radius = src.radius;
			dst.shadowIndex = *pLightIndex;
			dst.objectId = id;
			
			pLightIndex++;
		}
	}
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
			setTileType(dst, tileType);
		}
	}

	chunks.clear();
	dirtyChunks.clear();
	chunks.reserve(svState->map.chunks.size());
	dirtyChunks.reserve(svState->map.chunks.size());

	for (auto &pair : svState->map.chunks) {
		chunks[pair.key].dirty = true;
		chunks[pair.key].meshesDirty = true;
		dirtyChunks.push(pair.key);
	}

	objectTypes.clear();
	objectTypes.resize(svState->objectTypes.size);

	{
		uint32_t ix = 0;
		for (sv::GameObject &objectType : svState->objectTypes) {
			if (ix > 0) {
				convertObjectType(objectTypes[ix], objectType);
			}
			ix++;
		}
	}

	objects.clear();
	objects.reserve(svState->objects.size());

	{
		uint32_t ix = 0;
		for (auto &pair : svState->objects) {
			Object &object = objects[pair.key];
			ObjectType &type = objectTypes[pair.val.type];

			type.objects.insert(pair.key);
			updateObjectImp(*this, pair.key, type, object, pair.val);
			object.svObject = pair.val;
		}
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

	} else if (auto e = event->as<sv::EventUpdateTileType>()) {

		while (tileTypes.size <= e->index) tileTypes.push();
		setTileType(tileTypes[e->index], e->tileType);

	} else if (auto e = event->as<sv::EventUpdateChunk>()) {
		MapChunk &chunk = chunks[e->position];
		chunk.meshesDirty = true;
		if (!chunk.dirty) {
			chunk.dirty = true;
			dirtyChunks.push(e->position);
		}
	} else if (auto e = event->as<sv::EventUpdateObjectType>()) {
		while (objectTypes.size <= e->index) objectTypes.push();
		ObjectType &type = objectTypes[e->index];
		type.hasValidBounds = false;

		ObjectType prevType = type;
		convertObjectType(objectTypes[e->index], e->object);

		sf::Array<sv::Object> storedObjects;
		if (type.objects.size() > 0) {
			storedObjects.reserve(type.objects.size());
			sv::Object emptySvObject = { };
			for (uint32_t id : type.objects) {
				Object &object = objects[id];
				storedObjects.push(object.svObject);
				updateObjectImp(*this, id, prevType, object, object.svObject);
			}
		}

	} else if (auto e = event->as<sv::EventUpdateObject>()) {
		Object &object = objects[e->id];
		ObjectType &type = objectTypes[e->object.type];

		type.objects.insert(e->id);
		updateObjectImp(*this, e->id, type, object, e->object);
		object.svObject = e->object;

	} else if (auto e = event->as<sv::EventRemoveObject>()) {
		Object &object = objects[e->id];
		if (!object.svObject.type) return;
		ObjectType &type = objectTypes[object.svObject.type];

		updateObjectImp(*this, e->id, type, object, sv::Object());

		object = Object();
		type.objects.remove(e->id);
	} else {
		sf_failf("Unhandled event type: %u", event->type);
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

		if (chunk.meshesDirty) {
			if (!generateMapMeshes(chunk.meshes, *this, svState.map, chunk, chunkPos)) {
				continue;
			}
			chunk.meshesDirty = false;
		}

		if (!chunk.geometry.build(chunk.meshes, chunkPos)) {
			continue;
		}

		dirtyChunks.removeSwap(i--);
		chunk.dirty = false;
	}
}

void State::recreateTargets()
{
	shadowCache.recreateTargets();
}

void State::assetsReloaded()
{
	for (auto &pair : chunks) {
		cl::MapChunk &chunk = chunks[pair.key];
		chunk.meshesDirty = true;
		if (chunk.dirty) continue;
		chunk.dirty = true;
		dirtyChunks.push(pair.key);
	}
}

}
