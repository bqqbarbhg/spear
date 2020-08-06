#pragma once

#include "sf/Base.h"

namespace cl {

struct ClientState;

struct ClientSettings
{
	uint32_t shadowResolution = 128;
	uint32_t shadowRenderResolution = 512;
};

struct ClientGlobal
{
	ClientSettings settings;
	uint64_t frameIndex;
	sf::Box<ClientState> clientState;
};

extern thread_local ClientGlobal *clientGlobal;

}
