#include "game/client/ClientState.h"
#include "game/server/Message.h"

#include "MessageTransport.h"

#include "ext/bq_websocket.h"
#include "ext/bq_websocket_platform.h"
#include "ext/sokol/sokol_gfx.h"

#include "sp/Renderer.h"
#include "game/shader/GameShaders.h"
#include "game/shader/Postprocess.h"
#include "game/shader/Fxaa.h"

// TEMP
#include "sf/Frustum.h"
#include "game/shader/TestMesh.h"
#include "game/shader/TestSkin.h"
#include "ext/sokol/sokol_time.h"

static sf::Symbol serverName { "Server" };

static uint32_t playerIdCounter = 100;

struct ClientMain
{
	bqws_socket *ws;
	sf::Symbol name;

	sf::Box<sv::State> serverState;
	cl::State clientState;

	sf::Vec2i resolution;

	sp::Pipeline tonemapPipe;
	sp::Pipeline fxaaPipe;

	sp::RenderTarget mainTarget;
	sp::RenderTarget mainDepth;
	sp::RenderTarget tonemapTarget;
	sp::RenderTarget fxaaTarget;

	sp::RenderPass mainPass;
	sp::RenderPass tonemapPass;
	sp::RenderPass fxaaPass;

	sp::Pipeline tempMeshPipe;
	sp::Pipeline tempSkinnedMeshPipe;
};

ClientMain *clientInit(int port, const sf::Symbol &name)
{
	ClientMain *c = new ClientMain();
	c->name = name;

	gameShaders.load();

	{
        sf::SmallStringBuf<128> url;
        url.format("ws://localhost:%d", port);
        
		bqws_opts opts = { };
		opts.name = name.data;
        c->ws = bqws_pt_connect(url.data, NULL, &opts, NULL);
	}

	{
		sv::MessageJoin join;
		join.name = name;
		join.sessionId = 1;
		join.sessionSecret = 10;
		join.playerId = ++playerIdCounter;
		writeMessage(c->ws, &join, c->name, serverName);
	}

	c->tonemapPipe.init(gameShaders.postprocess, sp::PipeVertexFloat2);
	c->fxaaPipe.init(gameShaders.fxaa, sp::PipeVertexFloat2);

	{
		uint32_t flags = sp::PipeDepthWrite|sp::PipeIndex16|sp::PipeCullCCW;
		auto &d = c->tempMeshPipe.init(gameShaders.testMesh, flags);
		d.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
		d.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT3;
		d.layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT2;
	}

	{
		uint32_t flags = sp::PipeDepthWrite|sp::PipeIndex16|sp::PipeCullCCW;
		auto &d = c->tempSkinnedMeshPipe.init(gameShaders.skinnedMesh, flags);
		d.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
		d.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT3;
		d.layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT2;
		d.layout.attrs[3].format = SG_VERTEXFORMAT_UBYTE4;
		d.layout.attrs[4].format = SG_VERTEXFORMAT_UBYTE4N;
	}

	return c;
}

void clientQuit(ClientMain *client)
{
	bqws_close(client->ws, BQWS_CLOSE_NORMAL, NULL, 0);
}

static void recreateTargets(ClientMain *c, const sf::Vec2i &systemRes)
{
	c->resolution = systemRes;

	float scale = 1.0f;
	sf::Vec2i mainRes = sf::Vec2i(sf::Vec2(systemRes) * scale);

	int mainSamples = 4;
	sg_pixel_format mainFormat = SG_PIXELFORMAT_RGBA8;
	sg_pixel_format mainDepthFormat = SG_PIXELFORMAT_DEPTH_STENCIL;

	c->mainTarget.init("mainTarget", mainRes, mainFormat, mainSamples);
	c->mainDepth.init("mainDepth", mainRes, mainDepthFormat, mainSamples);
	c->tonemapTarget.init("tonemapTarget", mainRes, SG_PIXELFORMAT_RGBA8);
	c->fxaaTarget.init("fxaaTarget", mainRes, SG_PIXELFORMAT_RGBA8);

	c->mainPass.init("main", c->mainTarget, c->mainDepth);
	c->tonemapPass.init("tonemap", c->tonemapTarget);
	c->fxaaPass.init("fxaa", c->fxaaTarget);

	c->clientState.recreateTargets();

	// HACK
	{
		cl::PointLight &l = c->clientState.pointLights.push();
		l.position = sf::Vec3(0.0f, 4.0f, 0.0f);
		l.color = sf::Vec3(4.0f, 0.0f, 0.0f);
		l.radius = 16.0f;
		l.shadowIndex = 0;
	}

	{
		cl::PointLight &l = c->clientState.pointLights.push();
		l.position = sf::Vec3(0.0f, 4.0f, 0.0f);
		l.color = sf::Vec3(0.0f, 3.0f, 0.0f);
		l.radius = 16.0f;
		l.shadowIndex = 1;
	}

	{
		cl::PointLight &l = c->clientState.pointLights.push();
		l.position = sf::Vec3(0.0f, 4.0f, 0.0f);
		l.color = sf::Vec3(0.0f, 0.0f, 6.0f);
		l.radius = 16.0f;
		l.shadowIndex = 2;
	}

	{
		cl::PointLight &l = c->clientState.pointLights.push();
		l.position = sf::Vec3(0.0f, 2.0f, 0.0f);
		l.color = sf::Vec3(2.0f, 2.0f, 2.0f);
		l.radius = 16.0f;
		l.shadowIndex = 3;
	}
}

bool clientUpdate(ClientMain *c)
{
	if (bqws_is_closed(c->ws)) {
		return true;
	}

	bqws_update(c->ws);

	c->clientState.updateMapChunks(*c->serverState);

	while (bqws_msg *wsMsg = bqws_recv(c->ws)) {
		sf::Box<sv::Message> msg = readMessage(wsMsg);
		sf_assert(msg);

		if (auto m = msg->as<sv::MessageLoad>()) {
			m->state->refreshEntityTileMap();
			c->serverState = m->state;
			c->clientState.reset(m->state);
		} else if (auto m = msg->as<sv::MessageUpdate>()) {
			for (auto &event : m->events) {
				c->serverState->applyEvent(event);
				c->clientState.applyEvent(event);
			}
		}
	}

	return false;
}

void clientFree(ClientMain *client)
{
	delete client;
}

sg_image clientRender(ClientMain *c, const sf::Vec2i &resolution)
{
	if (resolution != c->resolution) {
		recreateTargets(c, resolution);
	}

	// HACK HACK
	{
		float t = (float)stm_sec(stm_now());
		uint32_t ix = 0;
		for (cl::PointLight &light : c->clientState.pointLights) {
			if (ix++ < 3) {
				light.position.x = sinf(t) * 5.0f;
				light.position.z = cosf(t) * 5.0f;
			}
			c->clientState.shadowCache.updatePointLight(c->clientState, light);
			t += sf::F_2PI / 3.0f;
		}
	}

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

		sf::Mat44 view = sf::mat::look(sf::Vec3(0.0f, 10.0f, 6.0f), sf::Vec3(0.0f, -1.0f, -0.5f));
		sf::Mat44 proj = sf::mat::perspectiveD3D(1.0f, (float)resolution.x/(float)resolution.y, 0.1f, 20.0f);
		sf::Mat44 viewProj = proj * view;

		TestMesh_Pixel_t pu = { };
		sf::Frustum frustum { viewProj, sg_query_features().origin_top_left ? 0.0f : -1.0f };
		for (auto &pair : c->clientState.chunks) {
			cl::MapChunkGeometry &chunkGeo = pair.val.geometry;
			if (!chunkGeo.main.vertexBuffer.buffer.id) continue;
			if (!frustum.intersects(chunkGeo.main.bounds)) continue;

			c->tempMeshPipe.bind();

			TestMesh_Transform_t transform;
			viewProj.writeColMajor44(transform.transform);
			sf::Mat44().writeColMajor44(transform.normalTransform);
			sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_TestMesh_Transform, &transform, sizeof(transform));

			pu.numLightsF = (float)c->clientState.pointLights.size;
			float (*dst)[4] = pu.lightData;
			for (cl::PointLight &light : c->clientState.pointLights) {
				dst[0][0] = light.position.x;
				dst[0][1] = light.position.y;
				dst[0][2] = light.position.z;
				dst[0][3] = light.radius;
				dst[1][0] = light.color.x;
				dst[1][1] = light.color.y;
				dst[1][2] = light.color.z;
				dst[1][3] = 0.0f;
				dst[2][0] = light.shadowMul.x;
				dst[2][1] = light.shadowMul.y;
				dst[2][2] = light.shadowMul.z;
				dst[2][3] = 0.0f;
				dst[3][0] = light.shadowBias.x;
				dst[3][1] = light.shadowBias.y;
				dst[3][2] = light.shadowBias.z;
				dst[3][3] = 0.0f;
				dst += 4;
			}

			sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_TestMesh_Pixel, &pu, sizeof(pu));
			sg_bindings bindings = { };
			bindings.vertex_buffers[0] = chunkGeo.main.vertexBuffer.buffer;
			bindings.index_buffer = chunkGeo.main.indexBuffer.buffer;
			bindings.fs_images[0] = c->clientState.shadowCache.shadowCache.image;
			sg_apply_bindings(&bindings);

			sg_draw(0, chunkGeo.main.numInidces, 1);
		}

		for (cl::Entity *entity : c->clientState.entities) {
			if (!entity) continue;

			if (cl::Character *chr = entity->as<cl::Character>()) {
				c->tempSkinnedMeshPipe.bind();

				if (!chr->model.isLoaded()) continue;
				cl::ModelInfo &modelInfo = chr->model->data;
				if (!modelInfo.modelRef.isLoaded()) continue;
				if (!modelInfo.skinRef.isLoaded()) continue;
				sp::Model *model = modelInfo.modelRef;
				sp::Sprite *sprite = modelInfo.skinRef;
				cl::AnimationInfo &animation = modelInfo.animations[0];
				if (!animation.modelRef.isLoaded()) continue;

				sp::Model *animModel = animation.modelRef;
				sp::Animation *anim = &animModel->animations[0];
				for (sp::Animation &a : animModel->animations) {
					if (a.name == animation.clip) {
						anim = &a;
						break;
					}
				}

				sf::SmallArray<sp::BoneTransform, sp::MaxBones> boneTransforms;
				sf::SmallArray<sf::Mat34, sp::MaxBones> boneWorld;
				boneTransforms.resizeUninit(model->bones.size);
				boneWorld.resizeUninit(model->bones.size);

				sf::Vec3 worldPos = sf::Vec3(chr->position.x, 0.0f, chr->position.y);
				sf::Mat34 world = sf::mat::translate(worldPos) * sf::mat::scale(modelInfo.scale);

				float animTime = fmodf((float)stm_sec(stm_now()), 80.0f/24.0f);
				sp::evaluateAnimation(animModel, boneTransforms, *anim, animTime);
				sp::boneTransformToWorld(model, boneWorld, boneTransforms, world);

				sp::Atlas *atlas = sprite->atlas;
				sf::Vec2 atlasSize = sf::Vec2((float)atlas->width, (float)atlas->height);
				sf::Vec2 uvSize = sprite->maxVert - sprite->minVert;
				sf::Vec2 size = sf::Vec2((float)sprite->width, (float)sprite->height);
				sf::Vec2 pos = sf::Vec2((float)sprite->x, (float)sprite->y) / atlasSize;

				sf::Vec2 scale = size / atlasSize / uvSize;
				sf::Vec2 offset = pos - sprite->minVert * scale;

				sf::Vec2 minUv = pos;
				sf::Vec2 maxUv = pos + size;

				pu.numLightsF = (float)c->clientState.pointLights.size;
				float (*dst)[4] = pu.lightData;
				for (cl::PointLight &light : c->clientState.pointLights) {
					dst[0][0] = light.position.x;
					dst[0][1] = light.position.y;
					dst[0][2] = light.position.z;
					dst[0][3] = light.radius;
					dst[1][0] = light.color.x;
					dst[1][1] = light.color.y;
					dst[1][2] = light.color.z;
					dst[1][3] = 0.0f;
					dst[2][0] = light.shadowMul.x;
					dst[2][1] = light.shadowMul.y;
					dst[2][2] = light.shadowMul.z;
					dst[2][3] = 0.0f;
					dst[3][0] = light.shadowBias.x;
					dst[3][1] = light.shadowBias.y;
					dst[3][2] = light.shadowBias.z;
					dst[3][3] = 0.0f;
					dst += 4;
				}

				sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_TestSkin_Pixel, &pu, sizeof(pu));

				TestSkin_VertexUniform_t vu;
				vu.color[0] = 1.0f;
				vu.color[1] = 1.0f;
				vu.color[2] = 1.0f;
				viewProj.writeColMajor44(vu.viewProj);

				vu.texScaleOffset[0] = scale.x;
				vu.texScaleOffset[1] = scale.y;
				vu.texScaleOffset[2] = offset.x;
				vu.texScaleOffset[3] = offset.y;

				TestSkin_FragUniform_t fragUniform;
				fragUniform.texMin[0] = minUv.x;
				fragUniform.texMin[1] = minUv.y;
				fragUniform.texMax[0] = maxUv.x;
				fragUniform.texMax[1] = maxUv.y;

				sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_TestSkin_FragUniform, &fragUniform, sizeof(fragUniform));

				for (sp::SkinMesh &mesh : model->skins) {
					TestSkin_Bones_t bones;
					for (uint32_t i = 0; i < mesh.bones.size; i++) {
						sp::MeshBone &meshBone = mesh.bones[i];
						sf::Mat34 transform = boneWorld[meshBone.boneIndex] * meshBone.meshToBone;
						memcpy(bones.bones[i * 3 + 0], transform.getRow(0).v, sizeof(sf::Vec4));
						memcpy(bones.bones[i * 3 + 1], transform.getRow(1).v, sizeof(sf::Vec4));
						memcpy(bones.bones[i * 3 + 2], transform.getRow(2).v, sizeof(sf::Vec4));
					}

					sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_TestSkin_VertexUniform, &vu, sizeof(vu));
					sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_TestSkin_Bones, &bones, sizeof(bones));

					sg_bindings binds = { };
					binds.vertex_buffers[0] = model->skinVertexBuffer;
					binds.index_buffer = model->skinIndexBuffer;
					binds.index_buffer_offset = mesh.indexBufferOffset * sizeof(uint16_t);
					binds.vertex_buffer_offsets[0] = mesh.vertexBufferOffset * sizeof(sp::SkinVertex);
					binds.fs_images[SLOT_TestSkin_albedo] = atlas->image;
					binds.fs_images[SLOT_TestSkin_shadowGrid] = c->clientState.shadowCache.shadowCache.image;
					sg_apply_bindings(&binds);

					sg_draw(0, mesh.numIndices, 1);
				}
			}
		}

		sp::endPass();
	}

#if 0
	{
		sp::beginPass(c->tonemapPass, nullptr);

		{
			c->tonemapPipe.bind();

			sg_bindings bindings = { };
			bindings.fs_images[SLOT_Postprocess_mainImage] = c->mainTarget.image;
			bindings.vertex_buffers[0] = gameShaders.fullscreenTriangleBuffer;
			sg_apply_bindings(&bindings);

			sg_draw(0, 3, 1);
		}

		sp::endPass();
	}

	{
		sp::beginPass(c->fxaaPass, nullptr);

		{
			c->fxaaPipe.bind();

			Fxaa_Pixel_t pixel;
			pixel.rcpTexSize = sf::Vec2(1.0f) / sf::Vec2(c->tonemapPass.resolution);
			sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_Fxaa_Pixel, &pixel, sizeof(pixel));

			sg_bindings bindings = { };
			bindings.fs_images[SLOT_Fxaa_Pixel] = c->tonemapTarget.image;
			bindings.vertex_buffers[0] = gameShaders.fullscreenTriangleBuffer;
			sg_apply_bindings(&bindings);

			sg_draw(0, 3, 1);
		}

		sp::endPass();
	}
#endif

	return c->mainTarget.image;
}

void clientDoMoveTemp(ClientMain *c)
{
	auto move = sf::box<sv::ActionMove>();
	move->entity = 1;
	move->position = sf::Vec2i(5, 5);

	sv::MessageAction action;
	action.action = move;
	writeMessage(c->ws, &action, c->name, serverName);
}
