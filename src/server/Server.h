#pragma once

#include "Message.h"

namespace sv {

struct Server;

struct ServerOpts
{
	int port = 4004;
	MessageEncoding messageEncoding;
};

Server *serverInit(const ServerOpts &opts);
void serverUpdate(Server *s);

}
