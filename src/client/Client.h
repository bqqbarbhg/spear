#pragma once

#include "sf/Symbol.h"
#include "sf/Vector.h"
#include "ext/sokol/sokol_defs.h"
#include "ext/sokol/sokol_app.h"

namespace cl {

struct Client;

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

Client *clientInit(int port, uint32_t sessionId, uint32_t sessionSecret);
void clientFree(Client *c);
void clientQuit(Client *c);
bool clientUpdate(Client *c, const ClientInput &input);
sg_image clientRender(Client *c);
void clientRenderGui(Client *c);
void clientAudio(Client *c, float *left, float *right, uint32_t numSamples, uint32_t sampleRate);

}