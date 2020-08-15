#pragma once

#include "client/System.h"
#include "server/ServerState.h"
#include "sf/HashSet.h"

namespace sf { struct Ray; }

namespace cl {

struct ClientState
{
	uint32_t localClientId = 0;
	Systems systems;

	ClientState(const SystemsDesc &desc);

	void applyEvent(const sv::Event &event);

	void editorPick(sf::Array<EntityHit> &hits, const sf::Ray &ray) const;

	void editorHighlight(uint32_t entityId, EditorHighlight type);

	void update(const sv::ServerState *svState, const FrameArgs &frameArgs);

	void renderShadows();

	void renderMain(const RenderArgs &renderArgs);

	void handleGui(const GuiArgs &guiArgs);

};

}
