#include "Client.h"

#include "ext/bq_websocket.h"
#include "sf/Array.h"
#include "server/Message.h"
#include "sp/Renderer.h"

#include "game/LocalServer.h"
#include "game/shader/GameShaders.h"
#include "game/shader/Fxaa.h"
#include "GameConfig.h"

#include "ext/bq_websocket.h"
#include "ext/bq_websocket_platform.h"

#include "sp/Asset.h"
#include "sp/Canvas.h"
#include "sp/Sprite.h"
#include "sp/Font.h"
#include "sp/RichText.h"

namespace cl {

struct Client
{
	// Communication
	sv::MessageEncoding messageEncoding;
	bqws_socket *ws;

	// Game state
	sf::Box<sv::ServerState> svState;

	// Render targets/passes
	sp::RenderTarget mainTarget;
	sp::RenderTarget mainDepth;
	sp::RenderTarget fxaaTarget;
	sp::RenderPass mainPass;
	sp::RenderPass tonemapPass;
	sp::RenderPass fxaaPass;

	// Pipelines
	sp::Pipeline fxaaPipe;

	// Options
	sf::Vec2i resolution;
	bool forceRecreateTargets = false;
	bool useFxaa = false;
	int msaaSamples = 4;
	float resolutionScale = 1.0f;

	// UI
	sp::Canvas canvas;
	sf::Vec2 uiResolution;
	sp::RichFont font {
		sp::FontRef{ "Assets/Gui/Font/NotoSans-Regular.ttf" },
		sp::FontRef{ "Assets/Gui/Font/NotoSans-Bold.ttf" },
		sp::FontRef{ "Assets/Gui/Font/NotoSans-Italic.ttf" },
		sp::FontRef{ "Assets/Gui/Font/NotoSans-BoldItalic.ttf" },
	};

	// Misc
	uint32_t reloadCount = 0;

};

static void sendMessage(Client &client, const sv::Message &msg)
{
	sf::SmallArray<char, 4096> data;
	encodeMessage(data, msg, client.messageEncoding);
	bqws_send_binary(client.ws, data.data, data.size);
}

static sf::Box<sv::Message> readMessageConsume(bqws_msg *wsMsg)
{
	sf::Box<sv::Message> msg = sv::decodeMessage(sf::slice(wsMsg->data, wsMsg->size));
	bqws_free_msg(wsMsg);
	return msg;
}

static void recreateTargets(Client *c, const sf::Vec2i &systemRes)
{
	c->resolution = systemRes;
	c->uiResolution.y = 720.0f;
	c->uiResolution.x = c->uiResolution.y * ((float)systemRes.x / (float)systemRes.y);

	float scale = c->resolutionScale;
	sf::Vec2i mainRes = sf::Vec2i(sf::Vec2(systemRes) * sqrtf(scale));

	int mainSamples = c->msaaSamples;
	sg_pixel_format mainFormat = SG_PIXELFORMAT_RGBA8;
	sg_pixel_format mainDepthFormat = SG_PIXELFORMAT_DEPTH_STENCIL;

	c->mainTarget.init("mainTarget", mainRes, mainFormat, mainSamples);
	c->mainDepth.init("mainDepth", mainRes, mainDepthFormat, mainSamples);
	c->mainPass.init("main", c->mainTarget, c->mainDepth);

	if (c->useFxaa) {
		c->fxaaTarget.init("fxaaTarget", mainRes, SG_PIXELFORMAT_RGBA8);
		c->fxaaPass.init("fxaa", c->fxaaTarget);
		if (!c->fxaaPipe.desc.shader.id) {
			c->fxaaPipe.init(gameShaders.fxaa, sp::PipeVertexFloat2);
		}
	} else {
		sf::reset(c->fxaaTarget);
		sf::reset(c->fxaaPass);
	}
}

void clientGlobalInit()
{
	gameShaders.load();
}

void clientGlobalCleanup()
{
}

void clientGlobalUpdate()
{
}

static void clientSocketError(void *user, bqws_socket *ws, bqws_error error)
{
	sf::debugPrint("Client socket error: %s (%u)\n", bqws_error_str(error), (unsigned)error);
	bqws_pt_error ptErr;
	if (bqws_pt_get_error(&ptErr)) {
		char buf[1024];
		bqws_pt_get_error_desc(buf, sizeof(buf), &ptErr);
		sf::debugPrint("Client IO error: %s\n", buf);
	}
}

Client *clientInit(int port, uint32_t sessionId, uint32_t sessionSecret)
{
	Client *c = new Client();

	{
        sf::SmallStringBuf<128> url;

#if defined(GAME_WEBSOCKET_URL)
        url = GAME_WEBSOCKET_URL;
		port = 80;
#else
        url.format("ws://localhost:%d", port);
#endif
        
		bqws_opts opts = { };
		opts.name = "Client";
		opts.error_fn = &clientSocketError;
		if (port > 0) {
			c->ws = bqws_pt_connect(url.data, NULL, &opts, NULL);
		} else {
			c->ws = localServerConnect(port, &opts, NULL);
		}
	}

	{
		sv::MessageJoin join;
		join.sessionId = sessionId;
		join.sessionSecret = sessionSecret;
		join.name = sf::Symbol("Client");
		sendMessage(*c, join);
	}

	return c;
}

void clientFree(Client *c)
{
}

void clientQuit(Client *c)
{
}

void handleMessage(Client *c, sv::Message &msg)
{
	if (auto m = msg.as<sv::MessageLoad>()) {
		c->svState = m->state;
	} else if (auto m = msg.as<sv::MessageUpdate>()) {
		for (sv::Event *event : m->events) {
			c->svState->applyEvent(*event);
		}
	}
}

static void updateCharacterPicking(Client *c, const ClientInput &input)
{
	sv::ServerState &svState = *c->svState;

	sp::RichTextStyle richStyle = { };
	richStyle.font = c->font;

	float x = 50.0f;
	float y = 50.0f;
	for (const auto &pair : svState.charactersToSelect) {
		sv::Prefab *tmplFab = svState.prefabs.find(pair.key);
		if (!tmplFab) continue;

		sv::CharacterTemplateComponent *tmplComp = tmplFab->findComponent<sv::CharacterTemplateComponent>();
		if (!tmplComp) continue;

		sv::Prefab *chrFab = svState.prefabs.find(tmplComp->characterPrefab);
		if (!chrFab) continue;

		sv::CharacterComponent *chrComp = chrFab->findComponent<sv::CharacterComponent>();
		if (!chrComp) continue;

		sp::SpriteRef sprite{chrComp->image};

		c->canvas.draw(sprite, sf::Vec2(x, y), sf::Vec2(200.0f));

		sf::String paragraphs[] = { tmplComp->description };

		sp::RichTextDesc desc = { };
		desc.style = &richStyle;
		desc.offset = sf::Vec2(x, y + 250.0f);
		desc.fontHeight = 30.0f;
		desc.baseColor = sf::Vec4(1.0f);
		desc.wrapWidth = 200.0f;
		sp::drawRichText(c->canvas, desc, paragraphs);

		x += 230.0f;
	}
}

bool clientUpdate(Client *c, const ClientInput &input)
{
	float dt = input.dt;
	if (bqws_is_closed(c->ws)) {
		return true;
	}

	if (input.resolution != c->resolution || c->forceRecreateTargets) {
		c->forceRecreateTargets = false;
		recreateTargets(c, input.resolution);
	}

	{
		uint32_t reloadCount = sp::Asset::getReloadCount();
		if (reloadCount != c->reloadCount) {
			c->reloadCount = reloadCount;
			sf::debugPrintLine("TODO: RELOAD");
		}
	}

	bqws_update(c->ws);
	while (bqws_msg *wsMsg = bqws_recv(c->ws)) {
		sf::Box<sv::Message> msg = readMessageConsume(wsMsg);

		if (msg) {
			handleMessage(c, *msg);
		}
	}

	c->canvas.clear();
	if (c->svState) {
		updateCharacterPicking(c, input);
	}

	c->canvas.prepareForRendering();

	return false;
}

sg_image clientRender(Client *c)
{
	{
		sg_pass_action action = { };
		action.colors[0].action = SG_ACTION_CLEAR;
		action.colors[0].val[0] = 0.01f;
		action.colors[0].val[1] = 0.01f;
		action.colors[0].val[2] = 0.01f;
		action.colors[0].val[3] = 1.0f;
		action.depth.action = SG_ACTION_CLEAR;
		action.depth.val = 1.0f;

		sp::beginPass(c->mainPass, &action);
		sp::endPass();
	}

	if (c->useFxaa) {
		sp::beginPass(c->fxaaPass, nullptr);

		{
			c->fxaaPipe.bind();

			Fxaa_Pixel_t pixel;
			pixel.rcpTexSize = sf::Vec2(1.0f) / sf::Vec2(c->mainPass.resolution);
			sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_Fxaa_Pixel, &pixel, sizeof(pixel));

			sg_bindings bindings = { };
			bindings.fs_images[SLOT_Fxaa_Pixel] = c->mainTarget.image;
			bindings.vertex_buffers[0] = gameShaders.fullscreenTriangleBuffer;
			sg_apply_bindings(&bindings);

			sg_draw(0, 3, 1);
		}

		sp::endPass();

		return c->fxaaTarget.image;
	}

	return c->mainTarget.image;
}

void clientRenderGui(Client *c)
{
	sp::CanvasRenderOpts opts = sp::CanvasRenderOpts::pixels(c->uiResolution);
	c->canvas.render(opts);
}

void clientAudio(Client *c, float *left, float *right, uint32_t numSamples, uint32_t sampleRate)
{
	memset(left, 0, numSamples * sizeof(float));
	memset(right, 0, numSamples * sizeof(float));
}

}
