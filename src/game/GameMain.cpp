#ifndef SP_NO_APP

#define _CRT_SECURE_NO_WARNINGS

#include "sp/GameMain.h"
#include "client/Client.h"
#include "server/Server.h"
#include "GameConfig.h"

#include "ext/bq_websocket.h"
#include "ext/bq_websocket_platform.h"
#include "ext/sokol/sokol_args.h"

cl::Client *g_client;
sv::Server *g_server;
extern bool g_hack_hd;

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

void spInit()
{
    #if defined(GAME_OVERRIDE_ARGS)
    static const char *overrideArgs[] = {
        GAME_OVERRIDE_ARGS
    };
    sp::commandLineArgs = sf::slice(overrideArgs);
    #endif
    
	{
		sargs_desc d = { };
		d.argv = (char**)sp::commandLineArgs.data;
		d.argc = sp::commandLineArgs.size;
		sargs_setup(&d);
	}

	{
		bqws_pt_init_opts opts = { };
		opts.ca_filename = "Misc/cacert.pem";
		bqws_pt_init(&opts);
	}

    int port = 4004;
	uint32_t sessionId = (uint32_t)atoi(sargs_value_def("id", "0"));
	uint32_t sessionSecret = (uint32_t)atoi(sargs_value_def("secret", "0"));

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

	clientGlobalInit();

    #if defined(GAME_GAME_URL)
        sp::ContentFile::addRelativeFileRoot(GAME_GAME_URL, "Game/");
    #else
    	sp::ContentFile::addRelativeFileRoot("Game", "Game/");
    #endif

	server = serverInit(port);
	sf::debugPrintLine("Server: %p", server);

	MainClient &client = clients.push();
	client.client = clientInit(port, sf::Symbol("Client 1"), sessionId, sessionSecret);

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

#endif
