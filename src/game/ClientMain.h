#pragma once

#include "sf/Symbol.h"
#include "ext/sokol/sokol_defs.h"

struct ClientMain;

ClientMain *clientInit(int port, const sf::Symbol &name);
void clientQuit(ClientMain *client);
bool clientUpdate(ClientMain *client);
void clientFree(ClientMain *client);
sg_image clientRender(ClientMain *client, const sf::Vec2i &resolution);

void clientDoMoveTemp(ClientMain *client);
