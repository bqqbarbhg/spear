#define _CRT_SECURE_NO_WARNINGS

#include "sp/GameMain.h"
#include "ClientMain.h"
#include "ServerMain.h"

void spConfig(sp::MainConfig &config)
{
	config.sappDesc.window_title = "Spear";
	config.sappDesc.sample_count = 1;

	config.sappDesc.width = 1200;
	config.sappDesc.height = 1080;

	config.sgDesc.buffer_pool_size = 10*1024;
	config.sgDesc.image_pool_size = 10*1024;
}

sp::FontRef font;
sp::Canvas canvas;

ServerMain *server;
sf::Array<ClientMain*> clients;

void spInit()
{
	server = serverInit();
	clients.push(clientInit(sf::Symbol("Client 1")));
}

void spCleanup()
{
}

void spEvent(const sapp_event *e)
{
	if (e->type == SAPP_EVENTTYPE_KEY_DOWN) {
		if (e->key_code == SAPP_KEYCODE_C) {
			sf::SmallStringBuf<64> name;
			name.format("Client %u", clients.size + 1);
			clients.push(clientInit(sf::Symbol(name)));
		} else if (e->key_code == SAPP_KEYCODE_Q) {
			ClientMain *c = clients.back();
			clientQuit(c);
		}
	}
}

void spFrame(float dt)
{
	serverUpdate(server);
	for (uint32_t i = 0; i < clients.size; i++) {
		ClientMain *client = clients[i];
		if (clientUpdate(client)) {
			clients.removeSwap(i--);
		}
	}

	sg_commit();
}
