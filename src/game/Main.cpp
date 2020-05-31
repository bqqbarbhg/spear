#ifndef SP_NO_APP

#define _CRT_SECURE_NO_WARNINGS

#include "sp/GameMain.h"
#include "ClientMain.h"
#include "ServerMain.h"

#include "sp/Renderer.h"
#include "sp/Font.h"
#include "sp/Canvas.h"
#include "game/shader/GameShaders.h"
#include "game/shader/Upscale.h"

#include "bq_websocket_platform.h"

#include "Processing.h"

#if SF_OS_EMSCRIPTEN
	#include <emscripten/html5.h>
#endif

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
	while (numX * numY < (int32_t)clients.size) {
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

int port;

void spInit()
{
    port = -1;

	#if SF_OS_EMSCRIPTEN
	{
		EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_get_current_context();
		emscripten_webgl_enable_extension(ctx, "EXT_texture_compression_s3tc");
		emscripten_webgl_enable_extension(ctx, "EXT_texture_compression_s3tc_srgb");
		emscripten_webgl_enable_extension(ctx, "EXT_texture_compression_rgtc");
		emscripten_webgl_enable_extension(ctx, "EXT_texture_compression_bptc");
	}
	#endif
    
    font.load(sf::Symbol("sp://OpenSans-Ascii.ttf"));
	gameShaders.load();

	clientGlobalInit();

	sp::ContentFile::addRelativeFileRoot("Build", "Assets/");
	sp::ContentFile::addRelativeFileRoot("Game", "Game/");
	// sp::ContentFile::addRelativeFileRoot("/data");
    // sp::ContentFile::addRelativeFileRoot("http://localhost:5000");

	server = serverInit(port);
	sf::debugPrintLine("Server: %p", server);

	MainClient &client = clients.push();
	client.client = clientInit(port, sf::Symbol("Client 1"));

	updateLayout();

	upscalePipe.init(gameShaders.upscaleFast, sp::PipeVertexFloat2);

	initializeProcessing();
}

void spCleanup()
{
	clientGlobalCleanup();
	closeProcessing();
}

static bool g_showStats;
static sf::Vec2 g_mousePos;

void spEvent(const sapp_event *e)
{
	if (e->type == SAPP_EVENTTYPE_KEY_DOWN && !e->key_repeat) {
		if (e->key_code == SAPP_KEYCODE_C) {
			sf::SmallStringBuf<64> name;
			name.format("Client %u", clients.size + 1);
			MainClient &client = clients.push();
			client.client = clientInit(port, sf::Symbol(name));
			updateLayout();
		} else if (e->key_code == SAPP_KEYCODE_Q) {
			MainClient &client = clients.back();
			clientQuit(client.client);
		} else if (e->key_code == SAPP_KEYCODE_M) {
			MainClient &client = clients.back();
			clientDoMoveTemp(client.client);
		} else if (e->key_code == SAPP_KEYCODE_F3) {
			g_showStats = !g_showStats;
		}
	} else if (e->type == SAPP_EVENTTYPE_MOUSE_MOVE) {
		g_mousePos.x = e->mouse_x;
		g_mousePos.y = e->mouse_y;
	} else if (e->type == SAPP_EVENTTYPE_RESIZED) {
		updateLayout();
	}
}

void spFrame(float dt)
{
	updateProcessing();

	if (server) serverUpdate(server);

	for (uint32_t i = 0; i < clients.size; i++) {
		MainClient &client = clients[i];

		ClientInput input;
		input.dt = dt;
		input.mousePosition = (g_mousePos - sf::Vec2(client.offset)) / sf::Vec2(client.resolution);
		input.resolution = client.resolution;

		if (clientUpdate(client.client, input)) {
			clientFree(client.client);
			clients.removeSwap(i--);
			updateLayout();
		}
	}
    
	canvas.clear();
	if (g_showStats) {
		float y = 100.0f;
		for (const sp::PassTime &time : sp::getPassTimes()) {
			sf::SmallStringBuf<128> text;
			text.format("%s: %.2fms", time.name.data, time.time * 1000.0);
			sp::TextDraw td;
			td.font = font;
			td.string = text;
			td.transform.m02 = 100.0f;
			td.transform.m12 = y; y += 60.0f;
			td.height = 60.0f;
			canvas.drawText(td);
		}
		
		canvas.prepareForRendering();
	}
   
    sp::Font::updateAtlasesForRendering();

	sp::beginFrame();

	for (MainClient &client : clients) {
		client.image = clientRender(client.client);
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

			clientRenderGui(client.client);
		}

		if (clients.size > 1) {
			sg_apply_viewport(0, 0, sapp_width(), sapp_height(), true);
		}

        canvas.render(sp::CanvasRenderOpts::windowPixels());

		sp::endPass();
	}

	sp::endFrame();

	sg_commit();
}

#endif
