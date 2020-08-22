#pragma once

#include "server/ServerState.h"

#include "client/System.h"

namespace sp { struct Canvas; }

namespace cl {

struct GameSystem : EntitySystem
{
	static sf::Box<GameSystem> create(const SystemsDesc &desc);

	virtual void writePersist(Systems &systems, ClientPersist &persist) = 0;

	virtual void updateCamera(FrameArgs &frameArgs) = 0;

	virtual void update(const sv::ServerState &svState, Systems &systems, const FrameArgs &frameArgs) = 0;
	virtual void applyEvent(Systems &systems, const sv::Event &event, bool immediate) = 0;

	virtual void getRequestedActions(sf::Array<sf::Box<sv::Action>> &actions) = 0;

	virtual void handleGui(Systems &systems, const GuiArgs &guiArgs) = 0;

};

}
