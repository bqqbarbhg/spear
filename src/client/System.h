#pragma once

#include "sf/Matrix.h"
#include "sf/Box.h"
#include "sf/UintMap.h"
#include "sf/Array.h"
#include "sf/HashSet.h"

#include "client/Transform.h"

namespace cl {

struct Systems;
struct EntityComponent;

enum class EditorHighlight
{
	Debug,
	Hint,
	Hover,
	Select,
};

struct TransformUpdate
{
	Transform previousTransform;
	Transform transform;
	sf::Mat34 entityToWorld;
};

struct System
{
};

struct EntitySystem : System
{
	virtual void updateTransform(Systems &systems, const EntityComponent &ec, const TransformUpdate &update) = 0;
	virtual void remove(Systems &systems, const EntityComponent &ec) = 0;

	virtual void editorHighlight(Systems &systems, const EntityComponent &ec, EditorHighlight type);
};

struct EntityComponent
{
	EntitySystem *system;
	uint32_t userId;
	uint16_t flags;
	uint8_t subsystemIndex;
	uint8_t componentIndex;
};

struct Entity
{
	enum Flags
	{
		UpdateTransform = 0x1,
	};

	uint32_t svId = 0;
	Transform transform;
	sf::SmallArray<EntityComponent, 4> components;
};

struct Entities
{
	sf::Array<Entity> entities;
	sf::Array<uint32_t> freeEntityIds;
	sf::UintMap svToEntity;

	uint32_t addEntity(uint32_t svId, const Transform &transform);
	void updateTransform(Systems &systems, uint32_t entityId, const Transform &transform);
	void removeEntity(Systems &systems, uint32_t entityId);

	void removeComponents(Systems &systems, uint32_t entityId);

	void addComponent(uint32_t entityId, EntitySystem *system, uint32_t userId, uint8_t subsystemIndex, uint8_t componentIndex, uint32_t flags);
};

struct AreaSystem;
struct ModelSystem;

enum class AreaGroup : uint32_t
{
	Model,
	Custom0,
};

struct Area
{
	enum Flags
	{
		Visibilty = 0x1,
		EditorPick = 0x2,
		Shadow = 0x4,
	};

	AreaGroup group;
	uint32_t flags;
	uint32_t userId;
};

struct VisibleAreas
{
	sf::Array<Area> areas;
	sf::Array<uint32_t> groups[(uint32_t)AreaGroup::Custom0];
	// TODO(?): sf::HashMap<uint32_t, sf::Array<uint32_t>> customGroups;

	sf_forceinline sf::Slice<const uint32_t> get(AreaGroup group) const {
		return groups[(uint32_t)group].slice();
	}
};

struct EntityHit
{
	uint32_t entityId = ~0u;
	float t = 0.0f;
};

struct Systems
{
	Entities entities;
	VisibleAreas visibleAreas;
	sf::Box<AreaSystem> area;
	sf::Box<ModelSystem> model;
};

struct FrameArgs
{
	uint64_t frameIndex = 0;
	float dt = 0.0f;

	sf::Vec3 cameraPosition;
	sf::Mat34 worldToView;
	sf::Mat44 viewToClip;
	sf::Mat44 worldToClip;
};

struct RenderArgs
{
	sf::Vec3 cameraPosition;
	sf::Mat34 worldToView;
	sf::Mat44 viewToClip;
	sf::Mat44 worldToClip;
};

}
