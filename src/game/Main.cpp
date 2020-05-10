#define _CRT_SECURE_NO_WARNINGS

#include "sp/GameMain.h"
#include "ClientMain.h"
#include "ServerMain.h"

#include "sp/Renderer.h"
#include "game/shader/GameShaders.h"
#include "game/shader/Upscale.h"

void spConfig(sp::MainConfig &config)
{
	config.sappDesc.window_title = "Spear";
	config.sappDesc.sample_count = 1;

	config.sappDesc.width = 1920;
	config.sappDesc.height = 1080;

	config.sgDesc.buffer_pool_size = 10*1024;
	config.sgDesc.image_pool_size = 10*1024;
	config.sgDesc.pass_pool_size = 10*1024;
}

sp::FontRef font;
sp::Canvas canvas;

struct MainClient
{
	ClientMain *client;
	sf::Vec2i offset;
	sf::Vec2i resolution;
	sg_image image;
};

ServerMain *server;
sf::Array<MainClient> clients;

sp::Pipeline upscalePipe;

static void updateLayout()
{
	sf::Vec2i systemRes = { sapp_width(), sapp_height() };
	int32_t numX = 1, numY = 1;
	while (numX * numY < clients.size) {
		if (numX <= numY) numX++; else numY++;
	}

	uint32_t index = 0;
	for (int32_t y = 0; y < numY; y++)
	for (int32_t x = 0; x < numX; x++)
	{
		if (index >= clients.size) break;

		sf::Vec2i min = systemRes * sf::Vec2i(x, y) / sf::Vec2i(numX, numY);
		sf::Vec2i max = systemRes * sf::Vec2i(x+1, y+1) / sf::Vec2i(numX, numY);
		clients[index].offset = min;
		clients[index].resolution = max - min;
		index++;
	}
}

void spInit()
{
	gameShaders.load();

	sp::ContentFile::addRelativeFileRoot("data");
	sp::ContentFile::addRelativeFileRoot("/data");

	server = serverInit();
	sf::debugPrintLine("Server: %p", server);

#if !SF_OS_WINDOWS
	MainClient &client = clients.push();
	client.client = clientInit(sf::Symbol("Client 1"));
#endif

	updateLayout();


	upscalePipe.init(gameShaders.upscaleFast, sp::PipeVertexFloat2);
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
			MainClient &client = clients.push();
			client.client = clientInit(sf::Symbol(name));
			updateLayout();
		} else if (e->key_code == SAPP_KEYCODE_Q) {
			MainClient &client = clients.back();
			clientQuit(client.client);
		} else if (e->key_code == SAPP_KEYCODE_M) {
			MainClient &client = clients.back();
			clientDoMoveTemp(client.client);
		}
	} else if (e->type == SAPP_EVENTTYPE_RESIZED) {
		updateLayout();
	}
}

void spFrame(float dt)
{
	if (server) serverUpdate(server);

	for (uint32_t i = 0; i < clients.size; i++) {
		MainClient &client = clients[i];
		if (clientUpdate(client.client)) {
			clientFree(client.client);
			clients.removeSwap(i--);
			updateLayout();
		}
	}

	sp::beginFrame();

	for (MainClient &client : clients) {
		client.image = clientRender(client.client, client.resolution);
	}

	{
		sp::beginDefaultPass(sapp_width(), sapp_height(), nullptr);

		upscalePipe.bind();

		for (MainClient &client : clients) {

			sg_apply_viewport(client.offset.x, client.offset.y, client.resolution.x, client.resolution.y, true);

			sg_bindings bindings = { };
			bindings.fs_images[SLOT_Upscale_tonemapImage] = client.image;
			bindings.vertex_buffers[0] = gameShaders.fullscreenTriangleBuffer;
			sg_apply_bindings(&bindings);

			sg_draw(0, 3, 1);
		}

		sp::endPass();
	}

	sp::endFrame();

	sg_commit();
}
