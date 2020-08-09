#pragma once

#include "client/System.h"
#include "server/ServerState.h"
#include "sf/HashSet.h"

namespace cl {

struct Prefab
{
	sv::Prefab s;
	sf::HashSet<uint32_t> entityIds;
};

struct PrefabKey { sf::Symbol& operator()(Prefab &t) { return t.s.name; } };

using PrefabMap = sf::ImplicitHashMap<Prefab, PrefabKey>;

struct ClientState
{
	Systems systems;
	PrefabMap prefabs;

	ClientState();

	void applyEvent(const sv::Event &event);

	void update(const FrameArgs &frameArgs);
	void renderMain(const RenderArgs &renderArgs);
};

}
