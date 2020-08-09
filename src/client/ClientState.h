#pragma once

#include "client/System.h"
#include "server/ServerState.h"
#include "sf/HashSet.h"

namespace sf { struct Ray; }

namespace cl {

struct PrefabKey { sf::Symbol& operator()(Prefab &t) { return t.s.name; } };

using PrefabMap = sf::ImplicitHashMap<Prefab, PrefabKey>;

struct ClientState
{
	uint32_t localClientId = 0;
	Systems systems;

	ClientState();

	void applyEvent(const sv::Event &event);

	void editorPick(sf::Array<EntityHit> &hits, const sf::Ray &ray) const;

	void editorHighlight(uint32_t entityId, EditorHighlight type);

	void update(const FrameArgs &frameArgs);
	void renderMain(const RenderArgs &renderArgs);

};

}
