#include "ClientState.h"

#include "sf/Random.h"

#include "game/shader/GameShaders.h"
#include "game/shader/MapShadow.h"

namespace cl {

static const sf::Symbol g_defaultTileMaterial { "Assets/Tiles/Default_Material/TileDefault" };

static void convertObject(Object &obj, const sv::GameObject &svObj)
{
	obj.sv = svObj;
	obj.mapMeshes.clear();
	obj.pointLights.clear();

	for (sv::Component *component : svObj.components) {
		if (sv::ModelComponent *c = component->as<sv::ModelComponent>()) {

			MapMesh &mesh = obj.mapMeshes.push();
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
				sf::mat::scale(c->stretch * c->scale * 0.01f));

			if (c->castShadows) {
				if (c->shadowModel) {
					mesh.shadowModel.load(c->shadowModel, props);
				} else {
					mesh.shadowModel = mesh.model;
				}
			}
		} else if (sv::PointLightComponent *c = component->as<sv::PointLightComponent>()) {
			PointLight &light = obj.pointLights.push();
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

static bool generateMapMeshes(sf::Array<cl::MapMesh> &meshes, cl::State &state, MapChunk &chunk, const sf::Vec2i &chunkPos)
{
	meshes.clear();

	uint32_t hash = sf::hash(chunkPos);
	sf::Random chunkRng { hash };

	sf::Vec2i origin = chunkPos * (int32_t)MapChunk::Size;

	for (sv::InstanceId instId : chunk.meshInstances) {
		Instance &inst = state.instances[instId];
		Object &obj = state.objects[inst.sv.objectId];

		sf::Mat34 transform = state.getInstanceTransform(inst.sv);
		for (MapMesh &mesh : obj.mapMeshes) {
			MapMesh &dst = meshes.push();
			dst = mesh;
			dst.transform = transform * mesh.transform;
		}
	}

	return true;
}

sf::Sphere State::getObjectBounds(Object &obj)
{
	if (obj.hasValidBounds) return obj.bounds;

	sf::Sphere bounds = { { }, -1.0f };
	float lightRadius = sf::length(sf::Vec3(0.1f));

	bool loading = false;
	for (MapMesh &mapMesh : obj.mapMeshes) {
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

	for (PointLight &light : obj.pointLights) {
		sf::Sphere sphere = { light.position, lightRadius };
		bounds = sf::sphereUnion(bounds, sphere);
	}

	if (!loading) {
		obj.bounds = bounds;
		obj.hasValidBounds = true;
	}

	return bounds;
}

sf::Vec3 State::getInstancePosition(const sv::InstancedObject &object)
{
	// TODO: Apply offset
	sf::Vec3 pos = { (float)object.x, 0.0f, (float)object.y };
	return pos;
}

sf::Mat34 State::getInstanceTransform(const sv::InstancedObject &object)
{
	sf::Vec3 pos = { (float)object.x, 0.0f, (float)object.y };
	return sf::mat::translate(pos) * sf::mat::rotateY((float)object.rotation * (sf::F_2PI / 256.0f));
}

void State::getObjectBounds(const Object &object, const sf::Mat34 &transform, sf::Array<sf::Mat34> &bounds)
{
	float lightRadius = 0.1f;
	for (const MapMesh &mapMesh : object.mapMeshes) {
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

	for (const PointLight &light : object.pointLights) {
		sf::Vec3 origin = sf::transformPoint(transform, light.position);
		bounds.push(sf::mat::translate(origin) * sf::mat::scale(lightRadius));
	}
}

sv::InstanceId State::pickInstance(float &outT, const sf::Ray &ray)
{
	sv::InstanceId minId = 0;
	float minT = HUGE_VALF;

	for (auto &pair : objects) {
		Object &obj = pair.val;
		sf::Sphere bounds = getObjectBounds(obj);

		for (sv::InstanceId id : obj.instances) {
			Instance &inst = instances[id];
			sf::Mat34 transform = getInstanceTransform(inst.sv);

			sf::Sphere objectBounds = { sf::transformPoint(transform, bounds.origin), bounds.radius };

			float t;
			if (sf::intesersectRay(t, ray, objectBounds) && t >= 0.0f) {
				if (t < minT) {
					sf::SmallArray<sf::Mat34, 32> obbs;
					getObjectBounds(obj, transform, obbs);

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

static void updateInstanceImp(State &state, sv::InstanceId instId, const Object &prevObj, const Instance &prevInst, const sv::InstancedObject &nextInst)
{
	bool added = (prevInst.sv.objectId == 0);
	bool removed = (nextInst.objectId == 0);
	Object &nextObj = state.objects[nextInst.objectId];
	sf::Vec2i tile = sf::Vec2i(nextInst.x, nextInst.y);
	sf::Vec2i chunkI = MapChunk::getChunk(tile);

	if (!added) {
		sf::Vec2i oldTile = sf::Vec2i(prevInst.sv.x, prevInst.sv.y);
		sf::Vec2i oldChunkI = MapChunk::getChunk(oldTile);
		if (prevObj.mapMeshes.size > 0 && (chunkI != oldChunkI || removed)) {
			MapChunk &chunk = state.chunks[oldChunkI];
			chunk.meshesDirty = true;
			chunk.meshInstances.remove(instId);
			if (!chunk.dirty) {
				chunk.dirty = true;
				state.dirtyChunks.push(oldChunkI);
			}
		}
		if (prevObj.pointLights.size > 0 && removed) {
			sf::Array<uint32_t> &lightIndices = state.pointLightMapping[instId];
			for (uint32_t index : lightIndices) {
				state.pointLights.removeSwap(index);
				if (index < state.pointLights.size) {
					PointLight &swapLight = state.pointLights[index];

					// HACK: Temp use shadow index
					swapLight.shadowIndex = index;

					sf::Array<uint32_t> &swapIndices = state.pointLightMapping[swapLight.objectId];
					for (uint32_t &swapRef : swapIndices) {
						if (swapRef == state.pointLights.size) {
							swapRef = index;
							break;
						}
					}
				}
			}
			state.pointLightMapping.remove(instId);
		}
	}

	if (nextObj.mapMeshes.size > 0) {
		MapChunk &chunk = state.chunks[chunkI];
		chunk.meshesDirty = true;
		chunk.meshInstances.insert(instId);
		if (!chunk.dirty) {
			chunk.dirty = true;
			state.dirtyChunks.push(chunkI);
		}
	}

	if (nextObj.pointLights.size > 0) {
		sf::Mat34 transform = state.getInstanceTransform(nextInst);
		sf::Array<uint32_t> &lightIndices = state.pointLightMapping[instId];
		lightIndices.reserve(nextObj.pointLights.size);
		while (lightIndices.size < nextObj.pointLights.size) lightIndices.push(~0u);
		uint32_t *pLightIndex = lightIndices.data;
		for (const PointLight &src : nextObj.pointLights) {
			if (*pLightIndex == ~0u) {
				*pLightIndex = state.pointLights.size;
				state.pointLights.push();
			}

			PointLight &dst = state.pointLights[*pLightIndex];
			dst.position = sf::transformPoint(transform, src.position);
			dst.color = src.color;
			dst.radius = src.radius;
			dst.shadowIndex = *pLightIndex;
			dst.objectId = instId;
			
			pLightIndex++;
		}
	}
}

void State::reset(sv::State *svState)
{
	chunks.clear();
	dirtyChunks.clear();

	objects.clear();
	objects.reserve(svState->objects.size());

	{
		for (auto &pair : svState->objects) {
			convertObject(objects[pair.key], pair.val);
		}
	}

	pointLights.clear();
	pointLightMapping.clear();

	instances.clear();
	instances.reserve(svState->instances.size());

	{
		uint32_t ix = 0;
		for (auto &pair : svState->instances) {
			Instance &inst = instances[pair.key];
			Object &obj = objects[pair.val.objectId];

			obj.instances.insert(pair.key);
			updateInstanceImp(*this, pair.key, obj, inst, pair.val);
			inst.sv = pair.val;
		}
	}
}

void State::applyEvent(sv::Event *event)
{
	if (auto e = event->as<sv::EventUpdateObject>()) {
		Object &obj = objects[e->id];
		obj.hasValidBounds = false;

		Object prevObj = obj;
		convertObject(obj, e->object);

		if (obj.instances.size() > 0) {
			for (sv::InstanceId instId : obj.instances) {
				Instance &inst = instances[instId];
				updateInstanceImp(*this, instId, prevObj, inst, inst.sv);
			}
		}

	} else if (auto e = event->as<sv::EventRemoveObject>()) {
		Object &obj = objects[e->id];
		sf_assert(obj.instances.size() == 0);

		objects.remove(e->id);

	} else if (auto e = event->as<sv::EventUpdateInstance>()) {
		Instance &inst = instances[e->id];
		Object &obj = objects[e->instance.objectId];

		obj.instances.insert(e->id);
		updateInstanceImp(*this, e->id, obj, inst, e->instance);
		inst.sv = e->instance;

	} else if (auto e = event->as<sv::EventRemoveInstance>()) {
		Instance &inst = instances[e->id];
		if (!inst.sv.objectId) return;
		Object &obj = objects[inst.sv.objectId];

		updateInstanceImp(*this, e->id, obj, inst, sv::InstancedObject());

		obj.instances.remove(e->id);
		instances.remove(e->id);
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
			if (!generateMapMeshes(chunk.meshes, *this, chunk, chunkPos)) {
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
