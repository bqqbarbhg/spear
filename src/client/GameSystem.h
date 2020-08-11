#pragma once

#include "server/ServerState.h"

#include "client/System.h"

namespace cl {

struct GameSystem : EntitySystem
{
	static sf::Box<GameSystem> create();

	virtual void update(const sv::ServerState &svState, const FrameArgs &frameArgs) = 0;


};

}
