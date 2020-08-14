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

#include "client/ClientState.h"
#include "client/EditorState.h"
#include "client/ParticleTexture.h"
#include "client/TileMaterial.h"
#include "client/MeshMaterial.h"

#include "game/DebugDraw.h"
#include "game/shader/Line.h"
#include "game/shader/Sphere.h"

#include "ext/imgui/imgui.h"

#include "sf/Random.h"

namespace cl {


struct DebugRenderHandles
{
	static constexpr const uint32_t MaxLinesPerFrame = 4096;
	static constexpr const uint32_t MaxSpheresPerFrame = 1024;

	bool initialized = false;
	sp::Buffer lineBuffer;
	sp::Buffer sphereVertexBuffer;
	sp::Buffer sphereIndexBuffer;
	sp::Buffer sphereInstanceBuffer;
	sp::Pipeline linePipe;
	sp::Pipeline spherePipe;

	void init()
	{
		if (initialized) return;
		initialized = true;

		{
			float phi = 1.61803398875f;
			sf::Vec3 sphereVertices[] = {
				{-1,phi,0}, {1,phi,0}, {-1,-phi,0}, {1,-phi,0}, 
				{0,-1,phi}, {0,1,phi}, {0,-1,-phi}, {0,1,-phi},
				{phi,0,-1}, {phi,0,1}, {-phi,0,-1}, {-phi,0,1},
			};
			uint16_t sphereIndices[] = {
				0, 1, 0, 5, 0, 7, 0, 10, 0, 11, 1, 5, 1, 7, 1, 8,
				1, 9, 2, 3, 2, 4, 2, 6, 2, 10, 2, 11, 3, 4, 3, 6,
				3, 8, 3, 9, 4, 5, 4, 9, 4, 11, 5, 9, 5, 11, 6, 7,
				6, 8, 6, 10, 7, 8, 7, 10, 8, 9, 10, 11
			};

			for (sf::Vec3 &v : sphereVertices) {
				v = sf::normalize(v);
			}

			sphereVertexBuffer.initVertex("Sphere vertex buffer", sf::slice(sphereVertices));
			sphereIndexBuffer.initIndex("Sphere index buffer", sf::slice(sphereIndices));
		}

		lineBuffer.initDynamicVertex("Line buffer", sizeof(sf::Vec3) * 2 * 2 * MaxLinesPerFrame);
		sphereInstanceBuffer.initDynamicVertex("Sphere instance buffer", sizeof(sf::Vec4) * 4 * MaxSpheresPerFrame);

		{
			uint32_t flags = sp::PipeDepthWrite;
			auto &d = linePipe.init(gameShaders.line, flags);
			d.primitive_type = SG_PRIMITIVETYPE_LINES;
			d.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
			d.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT3;
		}

		{
			uint32_t flags = sp::PipeDepthWrite | sp::PipeIndex16;
			auto &d = spherePipe.init(gameShaders.sphere, flags);
			d.primitive_type = SG_PRIMITIVETYPE_LINES;
			d.layout.buffers[1].step_func = SG_VERTEXSTEP_PER_INSTANCE;
			d.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
			for (uint32_t i = 1; i <= 4; i++) {
				d.layout.attrs[i].format = SG_VERTEXFORMAT_FLOAT4;
				d.layout.attrs[i].buffer_index = 1;
			}
		}
	}

	void render(const sf::Mat44 &worldToClip)
	{
		debugDrawFlipBuffers();
		DebugDrawData data = debugDrawGetData();

		sf::Slice<DebugLine> lines = data.lines.take(sf::min(MaxLinesPerFrame, (uint32_t)data.lines.size));
		sf::Slice<DebugSphere> spheres = data.spheres.take(sf::min(MaxSpheresPerFrame, (uint32_t)data.spheres.size));

		if (!lines.size && !spheres.size) return;

		if (!initialized) {
			init();
		}

		{
			sf::Array<sf::Vec3> lineData;
			lineData.resizeUninit(lines.size * 2 * 2);
			sf::Vec3 *dst = lineData.data;
			for (DebugLine &line : lines) {
				dst[0] = line.a;
				dst[1] = line.color;
				dst[2] = line.b;
				dst[3] = line.color;
				dst += 4;
			}
			sg_update_buffer(lineBuffer.buffer, lineData.data, (int)lineData.byteSize());
		}

		{
			sf::Array<sf::Vec4> sphereData;
			sphereData.resizeUninit(spheres.size * 4);
			sf::Vec4 *dst = sphereData.data;
			for (DebugSphere &sphere : spheres) {
				dst[0] = sf::Vec4(sphere.color, 0.0f);
				dst[1] = sphere.transform.getRow(0);
				dst[2] = sphere.transform.getRow(1);
				dst[3] = sphere.transform.getRow(2);
				dst += 4;
			}
			sg_update_buffer(sphereInstanceBuffer.buffer, sphereData.data, (int)sphereData.byteSize());
		}

		if (lines.size) {
			linePipe.bind();

			Line_Vertex_t vu;
			worldToClip.writeColMajor44(vu.worldToClip);
			sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_Line_Vertex, &vu, sizeof(vu));

			sg_bindings binds = { };
			binds.vertex_buffers[0] = lineBuffer.buffer;
			sg_apply_bindings(&binds);

			sg_draw(0, (int)lines.size * 2, 1);
		}

		if (spheres.size) {
			spherePipe.bind();

			Sphere_Vertex_t vu;
			worldToClip.writeColMajor44(vu.worldToClip);
			sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_Sphere_Vertex, &vu, sizeof(vu));

			sg_bindings binds = { };
			binds.index_buffer = sphereIndexBuffer.buffer;
			binds.vertex_buffers[0] = sphereVertexBuffer.buffer;
			binds.vertex_buffers[1] = sphereInstanceBuffer.buffer;
			sg_apply_bindings(&binds);

			sg_draw(0, 60, (int)spheres.size);
		}
	}
};

struct Client
{
	// Communication
	sv::MessageEncoding messageEncoding;
	bqws_socket *ws;

	// Update args
	cl::FrameArgs frameArgs;
	cl::RenderArgs mainRenderArgs;

	// Game state
	sf::Box<sv::ServerState> svState;
	sf::Box<cl::ClientState> clState;

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

	// Debug draw
	DebugRenderHandles debugRender;

	// Editor
	EditorState *editor = nullptr;

	// Misc
	uint32_t reloadCount = 0;

	// Errors
	uint32_t totalErrors = 0;
	sf::Array<sf::StringBuf> errors;

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
	MeshMaterial::globalInit();
	TileMaterial::globalInit();
	ParticleTexture::globalInit();
}

void clientGlobalCleanup()
{
	ParticleTexture::globalCleanup();
	TileMaterial::globalCleanup();
	MeshMaterial::globalCleanup();
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

static sf::Box<cl::ClientState> makeClientState(Client *c)
{
	cl::SystemsDesc desc;
	sf::getSecureRandom(desc.seed, sizeof(desc.seed));
	return sf::box<cl::ClientState>(desc);
}

Client *clientInit(int port, uint32_t sessionId, uint32_t sessionSecret)
{
	Client *c = new Client();

	c->clState = makeClientState(c);

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
	if (c->editor) {
		editorFree(c->editor);
		c->editor = nullptr;
	}
}

void clientQuit(Client *c)
{
}

static void handleLoadEvent(void *user, sv::Event &event)
{
	Client *c = (Client*)user;
	c->clState->applyEvent(event);
}

void handleMessage(Client *c, sv::Message &msg)
{
	if (auto m = msg.as<sv::MessageLoad>()) {
		c->svState = m->state;
		c->svState->localClientId = m->clientId;
		c->clState->localClientId = m->clientId;

		m->state->getAsEvents(&handleLoadEvent, c);

	} else if (auto m = msg.as<sv::MessageUpdate>()) {
		for (const sf::Box<sv::Event> &event : m->events) {

			if (c->editor) {
				if (editorPeekEventPre(c->editor, event)) {
					continue;
				}
			}

			c->svState->applyEvent(*event);
			c->clState->applyEvent(*event);
		}

	} else if (auto m = msg.as<sv::MessageQueryFilesResult>()) {

		if (c->editor) {
			editorAddQueryDir(c->editor, m->root, m->dir);
		}

	} else if (auto m = msg.as<sv::MessageErrorList>()) {

		if (c->errors.size > 400) {
			c->errors.removeOrdered(0, 200);
		}

		c->totalErrors += m->errors.size;
		c->errors.push(m->errors);

	}
}

void clientEvent(Client *c, const sapp_event *e)
{
	if (c->editor) {
		editorPeekSokolEvent(c->editor, e);
	}
	if (e->type == SAPP_EVENTTYPE_MOUSE_UP && e->mouse_button == SAPP_MOUSEBUTTON_RIGHT) {
		sapp_lock_mouse(false);
	}
}

bool clientUpdate(Client *c, const ClientInput &input)
{
	float dt = input.dt;
	if (bqws_is_closed(c->ws)) {
		return true;
	}

	// TODO: Don't always create editor
	for (const sapp_event &event : input.events) {
		if (event.type == SAPP_EVENTTYPE_KEY_DOWN && !ImGui::GetIO().WantCaptureKeyboard) {
			if (event.key_code == SAPP_KEYCODE_SPACE) {
				if (c->svState) {
					if (c->editor) {
						editorFree(c->editor);
						c->editor = nullptr;
					} else {
						c->editor = editorCreate(c->svState, c->clState);
					}
				}
			} else if (event.key_code == SAPP_KEYCODE_F5) {
				if (c->clState && c->svState) {
					if (c->editor) {
						editorPreRefresh(c->editor);
					}

					c->clState.reset();
					c->clState = makeClientState(c);
					c->clState->localClientId = c->svState->localClientId;
					c->svState->getAsEvents(&handleLoadEvent, c);

					if (c->editor) {
						editorPostRefresh(c->editor, c->clState);
					}
				}
			}
		}
	}

	c->frameArgs.gameTime += dt;
	c->frameArgs.frameIndex++;
	c->frameArgs.dt = dt;

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

	sf::Vec3 eye = sf::Vec3(0.0f, 5.0f, 3.0f) * 1.0f;
	sf::Mat34 worldToView = sf::mat::look(eye, sf::Vec3(0.0f, -1.0f, -0.4f));
	sf::Mat44 viewToClip = sf::mat::perspectiveD3D(1.3f, (float)sapp_width()/(float)sapp_height(), 1.0f, 100.0f);
	sf::Mat44 worldToClip = viewToClip * worldToView;

	c->mainRenderArgs.cameraPosition = eye;
	c->mainRenderArgs.worldToView = worldToView;
	c->mainRenderArgs.viewToClip = viewToClip;
	c->mainRenderArgs.worldToClip = worldToClip;
	c->mainRenderArgs.frustum = sf::Frustum(c->mainRenderArgs.worldToClip, sp::getClipNearW());

	c->frameArgs.mainRenderArgs = c->mainRenderArgs;

	if (c->editor) {
		EditorInput editorInput;
		editorInput.totalErrors = c->totalErrors;
		editorInput.errors = c->errors.slice();
		editorUpdate(c->editor, c->frameArgs, input, editorInput);

		EditorRequests &requests = editorPendingRequests(c->editor);

		if (requests.undo) {
			requests.undo = false;
			sv::MessageRequestEditUndo msg;
			sendMessage(*c, msg);
		}

		if (requests.redo) {
			requests.redo = false;
			sv::MessageRequestEditRedo msg;
			sendMessage(*c, msg);
		}

		for (sf::StringBuf &path : requests.queryDirs) {
			sv::MessageQueryFiles msg;
			msg.root = path;
			sendMessage(*c, msg);
		}
		requests.queryDirs.clear();

		for (sf::Array<sf::Box<sv::Edit>> &bundle : requests.edits) {
			if (bundle.size == 0) continue;

			sv::MessageRequestEdit msg;
			msg.edits = std::move(bundle);
			sendMessage(*c, msg);
		}
		requests.edits.clear();
	}

	c->clState->update(c->svState, c->frameArgs);

#if 0
#endif

	c->canvas.clear();

	c->canvas.prepareForRendering();

	return false;
}

sg_image clientRender(Client *c)
{
	c->clState->renderShadows();

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

		c->clState->renderMain(c->mainRenderArgs);

		c->debugRender.render(c->mainRenderArgs.worldToClip);

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
