#ifndef SP_NO_APP

#define _CRT_SECURE_NO_WARNINGS

#include "sp/GameMain.h"
#include "client/Client.h"
#include "server/Server.h"

#include "sp/Renderer.h"
#include "sp/Font.h"
#include "sp/Canvas.h"
#include "sp/Args.h"
#include "game/shader/GameShaders.h"
#include "game/shader/Upscale.h"
#include "ext/sokol/sokol_imgui.h"
#include "ext/sokol/sokol_args.h"
#include "ext/imgui/imgui.h"

#include "GameConfig.h"

#include "bq_websocket_platform.h"

#include "Processing.h"

#include "sf/Float4.h"

#include "sf/Mutex.h"

#if SF_OS_EMSCRIPTEN
	#include <emscripten/emscripten.h>
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

	config.saudioDesc.num_channels = 2;
}

sp::FontRef font;
sp::Canvas canvas;

struct MainClient
{
	cl::Client *client;
	sf::Vec2i offset;
	sf::Vec2i resolution;
	sg_image image;
};

sv::Server *server;
sf::Array<MainClient> clients;
sf::StaticMutex clientMutex;

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

static int port;
static uint32_t sessionId;
static uint32_t sessionSecret;

extern bool g_hack_hd;

static void *imguiAlloc(size_t size, void*) { return sf_malloc(size); }
static void imguiFree(void *ptr, void*) { return sf_free(ptr); }

void spInit()
{
    #if defined(GAME_OVERRIDE_ARGS)
    static const char *overrideArgs[] = {
        GAME_OVERRIDE_ARGS
    };
    sp::commandLineArgs = sf::slice(overrideArgs);
    #endif

	ImGui::SetAllocatorFunctions(&imguiAlloc, &imguiFree);

	sf::MutexGuard mg(clientMutex);
    
	{
		sargs_desc d = { };
		d.argv = (char**)sp::commandLineArgs.data;
		d.argc = (int)sp::commandLineArgs.size;
		sargs_setup(&d);
	}

	{
		bqws_pt_init_opts opts = { };
		opts.ca_filename = "Misc/cacert.pem";
		bqws_pt_init(&opts);
	}

    port = 4004;
	sessionId = (uint32_t)atoi(sargs_value_def("id", "0"));
	sessionSecret = (uint32_t)atoi(sargs_value_def("secret", "0"));

	#if SF_OS_EMSCRIPTEN
	{
		EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_get_current_context();
		emscripten_webgl_enable_extension(ctx, "EXT_texture_compression_s3tc");
		emscripten_webgl_enable_extension(ctx, "EXT_texture_compression_s3tc_srgb");
		emscripten_webgl_enable_extension(ctx, "EXT_texture_compression_rgtc");
		emscripten_webgl_enable_extension(ctx, "EXT_texture_compression_bptc");
	}
	#endif

	if (sargs_boolean("cdn")) {
		#if defined(GAME_CDN_URL)
			sp::ContentFile::addRelativeFileRoot(GAME_CDN_URL, "Assets/");
		#endif
	} else {
		sp::ContentFile::addRelativeFileRoot("Build", "Assets/");
	}

	if (sargs_boolean("hd")) {
		g_hack_hd = true;
	}
    
    font.load(sf::Symbol("sp://OpenSans-Ascii.ttf"));

	gameShaders.load();

	cl::clientGlobalInit();

    #if defined(GAME_GAME_URL)
        sp::ContentFile::addRelativeFileRoot(GAME_GAME_URL, "Game/");
    #else
    	sp::ContentFile::addRelativeFileRoot("Game", "Game/");
    #endif

	sv::ServerOpts serverOpts;
	serverOpts.port = port;
	server = sv::serverInit(serverOpts);
	sf::debugPrintLine("Server: %p", server);

	MainClient &client = clients.push();
	client.client = cl::clientInit(port, sessionId, sessionSecret);

	{
		simgui_desc_t d = { };
		d.ini_filename = "";
		d.dpi_scale = sapp_dpi_scale();
		simgui_setup(&d);
	}

	updateLayout();

	upscalePipe.init(gameShaders.upscaleFast, sp::PipeVertexFloat2);

	{
		ProcessingDesc desc = { };
		desc.localProcessing = true;
		initializeProcessing(desc);
	}
}

void spCleanup()
{
	font.reset();
	sf::reset(canvas);
	upscalePipe.reset();

	{
		sf::MutexGuard mg(clientMutex);
		for (MainClient &mc : clients) {
			cl::clientFree(mc.client);
		}
		clients.clear();
	}

	// TODO: Proper reset
	sf::reset(gameShaders);

	simgui_shutdown();
	cl::clientGlobalCleanup();
	closeProcessing();
}

static bool g_showStats;
static sf::Vec2 g_mousePos;
static sf::Array<sapp_event> g_events;

void spEvent(const sapp_event *e)
{
	simgui_handle_event(e);

	if (!e->key_repeat) {
		g_events.push(*e);
	}

	if (e->type == SAPP_EVENTTYPE_KEY_DOWN && !e->key_repeat && !ImGui::GetIO().WantCaptureKeyboard) {

#if 0
		if (e->key_code == SAPP_KEYCODE_C) {
			sf::MutexGuard mg(clientMutex);
			sf::SmallStringBuf<64> name;
			name.format("Client %u", clients.size + 1);
			MainClient &client = clients.push();
			client.client = clientInit(port, sf::Symbol(name), sessionId, sessionSecret);
			updateLayout();
		} else if (e->key_code == SAPP_KEYCODE_Q) {
			sf::MutexGuard mg(clientMutex);
			MainClient &client = clients.back();
			clientQuit(client.client);
		} else if (e->key_code == SAPP_KEYCODE_M) {
			MainClient &client = clients.back();
			clientDoMoveTemp(client.client);
		}
#endif

		if (e->key_code == SAPP_KEYCODE_F3) {
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
	simgui_new_frame(sapp_width(), sapp_height(), dt);

	updateProcessing();

	if (server) sv::serverUpdate(server);

	cl::clientGlobalUpdate();

	for (uint32_t i = 0; i < clients.size; i++) {
		MainClient &client = clients[i];

		cl::ClientInput input;
		input.dt = dt;
		input.mousePosition = (g_mousePos - sf::Vec2(client.offset)) / sf::Vec2(client.resolution);
		input.resolution = client.resolution;
		input.events = g_events;

		// TODO: Behind a debug mode
		if (!ImGui::GetIO().WantCaptureKeyboard && ImGui::IsKeyDown(SAPP_KEYCODE_TAB)) {
			input.dt *= 0.1f;
		}

		if (clientUpdate(client.client, input)) {
			sf::MutexGuard mg(clientMutex);
			cl::clientFree(client.client);
			clients.removeSwap(i--);
			updateLayout();
		}

		g_events.clear();
	}
    
	canvas.clear();

	{
		sf::SmallArray<ProcessingAsset, 32> processingAssets;
		queryProcessingAssets(processingAssets);

		float y = 40.0f;
		for (ProcessingAsset &asset : processingAssets) {
			sf::SmallStringBuf<128> text;
			text.format("%s %u/%u", asset.name.data, asset.tasksDone, asset.tasksPending);
			sp::TextDraw td;
			td.font = font;
			td.string = text;
			td.transform.m02 = 40.0f;
			td.transform.m12 = y; y += 18.0f;
			td.height = 18.0f;
			canvas.drawText(td);
		}
	}

	if (g_showStats) {
		float y = 50.0f;
		for (const sp::PassTime &time : sp::getPassTimes()) {
			sf::SmallStringBuf<128> text;
			text.format("%s: %.2fms", time.name.data, time.time * 1000.0);
			sp::TextDraw td;
			td.font = font;
			td.string = text;
			td.transform.m02 = 100.0f;
			td.transform.m12 = y; y += 30.0f;
			td.height = 30.0f;
			canvas.drawText(td);
		}
	}

	canvas.prepareForRendering();
   
    sp::Font::updateAtlasesForRendering();

	sp::beginFrame();

	for (MainClient &client : clients) {
		client.image = cl::clientRender(client.client);
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

			cl::clientRenderGui(client.client);
		}

		if (clients.size > 1) {
			sg_apply_viewport(0, 0, sapp_width(), sapp_height(), true);
		}

        canvas.render(sp::CanvasRenderOpts::windowPixels());

		simgui_render();

		sp::endPass();
	}

	sp::endFrame();

	sg_commit();
}

static sf::Array<float> mixChannels[2];

void spAudio(float* buffer, int numFrames, int numChannels)
{
	sf_assert(numFrames % 4 == 0);
	uint32_t sampleRate = saudio_sample_rate();
	sf::MutexGuard mg(clientMutex);

	if (clients.size == 0) {
		memset(buffer, 0, sizeof(float) * numFrames * numChannels);
		return;
	}

	mixChannels[0].resizeUninit(numFrames);
	mixChannels[1].resizeUninit(numFrames);

	cl::clientAudio(clients[0].client, mixChannels[0].data, mixChannels[1].data, numFrames, sampleRate);

	float *dst = buffer, *srcL = mixChannels[0].data, *srcR = mixChannels[1].data;
	for (uint32_t i = 0; i < (uint32_t)numFrames / 4; i++) {
		sf::Float4 l = sf::Float4::loadu(srcL);
		sf::Float4 r = sf::Float4::loadu(srcR);
		sf::Float4 d0, d1;

		sf::Float4::interleave2(d0, d1, l, r);

		d0.storeu(dst + 0);
		d1.storeu(dst + 4);

		srcL += 4;
		srcR += 4;
		dst += 8;
	}

}

#endif

bool g_hack_hd;
