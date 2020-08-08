#include "ClientState.h"

#include "AreaState.h"
#include "LightState.h"
#include "MeshState.h"

namespace cl {

static VisualTransform getPropTransform(const sv::PropTransform &transform)
{
	VisualTransform ret;
	ret.position = sf::Vec3((float)transform.tile.x, 0.0f, (float)transform.tile.y) + transform.visualOffset;
	ret.rotation = sf::eulerAnglesToQuat(transform.visualRotation);
	ret.scale = sf::Vec3(transform.scale);
	return ret;
}

ClientState::ClientState()
{
	areaState = AreaState::create();
	lightState = LightState::create();
	meshState = MeshState::create();
}

void ClientState::registerEntity(uint32_t entityId, Entity::Mask mask)
{
	Entity *entity = entities.find(entityId);
	sf_assert(entity);
	entity->mask |= (uint32_t)mask;
}

void ClientState::addEntity(uint32_t entityId, const sf::Symbol &prefabName, const VisualTransform &transform)
{
	Entity &entity = entities[entityId];
	entity.id = entityId;
	entity.prefabName = prefabName;
	entity.transform = transform;

	Prefab *prefab = prefabs.find(prefabName);
	if (!prefab) return;

	for (const sf::Box<sv::Component> &comp : prefab->s.components) {

		if (const auto *c = comp->as<sv::ModelComponent>()) {
			meshState->addMesh(entityId, comp.cast<sv::ModelComponent>());
		}

	}
}

void ClientState::setEntityTransform(uint32_t entityId, const VisualTransform &transform)
{
	Entity *entity = entities.find(entityId);
	sf_assert(entity);

	VisualTransform normalized = transform;
	normalized.rotation = sf::normalize(transform.rotation);

	entity->transform = normalized;

	sf::Mat34 matrix = normalized.asMatrix();

	if (entity->mask & Entity::Mesh) {
		meshState->updateEntityTransform(entityId, transform, matrix);
	}
	if (entity->mask & Entity::Light) {
		lightState->updateEntityTransform(entityId, transform, matrix);
	}
	if (entity->mask & Entity::Area) {
		areaState->updateEntityTransform(entityId, transform, matrix);
	}
}

void ClientState::addEntityInterpolation(uint32_t entityId, const VisualTransform &dst, const EntityInterpolationOpts &opts)
{
	// Special case: Instant
	if (opts.duration <= 0.001f) {
		entityInterpolations.remove(entityId);
		setEntityTransform(entityId, dst);
		return;
	}

	Entity *entity = entities.find(entityId);
	sf_assert(entity);

	auto res = entityInterpolations.insert(entityId);
	EntityInterpolation &interp = res.entry;

	float speed = 1.0f / opts.duration;

	VisualTransform velocity;
	if (!res.inserted) {
		velocity = interp.spline.derivative(interp.t) * (interp.speed / speed);
	} else {
		interp.entityId = entityId;
	}

	VisualTransform delta = dst - entity->transform;

	interp.t = 0.0f;
	interp.speed = speed;
	interp.spline.p0 = entity->transform;
	interp.spline.d0 = delta * opts.velocityBegin;
	interp.spline.p1 = dst;
	interp.spline.d1 = delta * opts.velocityEnd;

	interp.spline.d0 += velocity * opts.velocityInherit;
}

void ClientState::updateEntityInterpolations(float dt)
{
	for (uint32_t ix = 0; ix < entityInterpolations.size(); ix++) {
		EntityInterpolation &interp = entityInterpolations.data[ix];

		interp.t += dt * interp.speed;
		if (interp.t >= 1.0f) {
			setEntityTransform(interp.entityId, interp.spline.p1);
			entityInterpolations.removeAt(&interp);
			ix--;
		} else {
			VisualTransform transform = interp.spline.evaluate(interp.t);
			setEntityTransform(interp.entityId, transform);
		}
	}
}

void ClientState::applyEvent(const sv::Event &event)
{
	if (const auto *e = event.as<sv::LoadPrefabEvent>()) {
		Prefab &prefab = prefabs[e->prefab.name];
		prefab.s = e->prefab;
	} else if (const auto *e = event.as<sv::AddPropEvent>()) {
		VisualTransform transform = getPropTransform(e->prop.transform);
		addEntity(e->prop.id, e->prop.prefabName, transform);
	} else if (const auto *e = event.as<sv::MovePropEvent>()) {
		VisualTransform transform = getPropTransform(e->transform);
		EntityInterpolationOpts opts;
		opts.duration = ((float)rand() / (float)RAND_MAX) * 3.0f + 0.5f;
		opts.velocityBegin = 0.0f;
		addEntityInterpolation(e->propId, transform, opts);
	}
}

void ClientState::updateAssetLoading()
{
	meshState->updatePendingMeshes();
}

void ClientState::updateVisibility(const sf::Frustum &frustum)
{
	for (uint32_t i = 0; i < 3; i++) {
		areaState->optimizeSpatialNodes();
	}

	visibleNodes.clear();
	areaState->querySpatialNodesFrustum(visibleNodes, frustum);
	areaState->updateMainVisibility(visibleNodes, frustum);

	lightState->updateVisibility();
}

void ClientState::renderShadows(const RenderArgs &args)
{
}

void ClientState::renderMain(const RenderArgs &args)
{
	meshState->renderMeshses(args);
}

}
