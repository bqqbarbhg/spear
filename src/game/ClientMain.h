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
void clientGlobalUpdate();

ClientMain *clientInit(int port, const sf::Symbol &name, uint32_t sessionId, uint32_t sessionSecret);
void clientQuit(ClientMain *client);
bool clientUpdate(ClientMain *client, const ClientInput &input);
void clientFree(ClientMain *client);
sg_image clientRender(ClientMain *client);
void clientRenderGui(ClientMain *client);

void clientDoMoveTemp(ClientMain *client);

void clientAudio(ClientMain *c, float *left, float *right, uint32_t numSamples, uint32_t sampleRate);
