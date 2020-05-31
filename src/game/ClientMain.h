#pragma once

#include "sf/Symbol.h"
#include "sf/Vector.h"
#include "ext/sokol/sokol_defs.h"
#include "ext/sokol/sokol_app.h"

struct ClientMain;

struct ClientInput
{
	float dt;
	sf::Vec2 mousePosition;
	sf::Vec2i resolution;
	sf::Slice<sapp_event> events;
};

void clientGlobalInit();
void clientGlobalCleanup();

ClientMain *clientInit(int port, const sf::Symbol &name);
void clientQuit(ClientMain *client);
bool clientUpdate(ClientMain *client, const ClientInput &input);
void clientFree(ClientMain *client);
sg_image clientRender(ClientMain *client);
void clientRenderGui(ClientMain *client);

void clientDoMoveTemp(ClientMain *client);
