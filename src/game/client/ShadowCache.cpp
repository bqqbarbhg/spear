#include "ShadowCache.h"
#include "ClientState.h"

#include "game/shader/GameShaders.h"
#include "game/shader/ShadowGrid.h"

// HACK

#include "ext/sokol/sokol_time.h"

namespace cl {

void ShadowCache::recreateTargets()
{
	// TODO: Make this lazy

	{
		sg_image_desc d = { };
		d.label = "shadowCache";
		d.pixel_format = SG_PIXELFORMAT_R8;
		#if CL_SHADOWCACHE_USE_ARRAY
			d.type = SG_IMAGETYPE_ARRAY;
		#else
			d.type = SG_IMAGETYPE_3D;
		#endif
		d.width = (int)(cacheTileExtent * cacheNumTilesX);
		d.height = (int)(cacheTileExtent * cacheNumTilesX);
		d.depth = (int)cacheTileSlices;
		d.num_mipmaps = 1;
		d.min_filter = d.mag_filter = SG_FILTER_LINEAR;
		d.wrap_u = d.wrap_v = d.wrap_w = SG_WRAP_CLAMP_TO_EDGE;
		d.bqq_copy_target = true;
		shadowCache.init(d);
	}

	{
		sg_image_desc d = { };
		d.label = "depthRenderCube";
		d.render_target = true;
		d.pixel_format = SG_PIXELFORMAT_R16F;
		d.type = SG_IMAGETYPE_CUBE;
		d.width = (int)depthRenderExtent;
		d.height = (int)depthRenderExtent;
		d.depth = (int)(cacheTileSlices);
		d.num_mipmaps = 1;
		d.min_filter = d.mag_filter = SG_FILTER_LINEAR;
		d.wrap_u = d.wrap_v = d.wrap_w = SG_WRAP_REPEAT;
		depthRenderCube.init(d);
	}

	shadowCacheRender.init("shadowCacheRender", sf::Vec2i(cacheTileExtent * cacheTileSlices, cacheTileExtent), SG_PIXELFORMAT_R8);
	shadowCachePass.init("shadowCache", shadowCacheRender);

	depthRenderDepth.init("depthRenderDepth", sf::Vec2i((uint32_t)depthRenderExtent), SG_PIXELFORMAT_DEPTH);

	{
		sp::FramebufferDesc fbDesc;
		fbDesc.colorFormat = SG_PIXELFORMAT_R16F;
		fbDesc.depthFormat = depthRenderDepth.format;
		sg_pass_desc passDesc = { };
		sf::Vec2i passRes = depthRenderDepth.resolution;
		for (uint32_t i = 0; i < 6; i++) {
			sf::SmallStringBuf<128> name;
			name.format("shadowDepthPass face %u", i);
			passDesc.label = name.data;
			passDesc.color_attachments[0].image = depthRenderCube.image;
			passDesc.color_attachments[0].face = (int)i;
			passDesc.depth_stencil_attachment.image = depthRenderDepth.image;
			depthRenderPass[i].init(passDesc, passRes, fbDesc);
		}
	}
}

void ShadowCache::updatePointLight(State &cs, PointLight &light)
{
	sf::Vec3 cubeBasis[][2] = {
		{ { +1,0,0 }, { 0,+1,0 } },
		{ { -1,0,0 }, { 0,+1,0 } },
		{ { 0,+1,0 }, { 0,0,+1 } },
		{ { 0,-1,0 }, { 0,0,-1 } },
		{ { 0,0,-1 }, { 0,+1,0 } },
		{ { 0,0,+1 }, { 0,+1,0 } },
	};

	sg_pass_action action = { };
	action.colors[0].action = SG_ACTION_CLEAR;
	action.colors[0].val[0] = light.radius;
	action.colors[0].val[1] = light.radius;
	action.colors[0].val[2] = light.radius;
	action.colors[0].val[3] = light.radius;
	action.depth.action = SG_ACTION_CLEAR;
	action.depth.val = 1.0f;

	float clipNearW = sg_query_features().origin_top_left ? -1.0f : 0.0f;
	for (uint32_t side = 0; side < 6; side++) {
		RenderShadowArgs args;

		const sf::Vec3 *basis = cubeBasis[side];
		sf::Mat34 view = sf::mat::look(light.position, basis[0], basis[1]);
		sf::Mat44 proj = sf::mat::perspectiveD3D(sf::F_PI/2.0f, 1.0f, 0.1f, light.radius);
		args.worldToClip = proj * view;
		args.cameraPosition = light.position;
		args.frustum = sf::Frustum(args.worldToClip, clipNearW);

		sp::beginPass(depthRenderPass[side], &action);
		cs.renderShadows(args);
		sp::endPass();
	}

	float radius = light.radius;
	float height = 4.0f;

	sf::Vec3 f = sf::Vec3((float)cacheTileExtent, (float)cacheTileSlices, (float)cacheTileExtent) / (light.radius*2.0f);
	// f = sf::Vec3(1.0f);
	sf::Vec3 p = light.position * f;
	p.x = p.x - floorf(p.x);
	// p.y = p.y - floorf(p.y);
	p.z = p.z - floorf(p.z);
	sf::Vec3 volumeOrigin = sf::Vec3(-radius, -1.0f, -radius) - p / f;
	sf::Vec3 volumeExtent = sf::Vec3(radius*2.0f, height, radius*2.0f);

	uint32_t offsetX = light.shadowIndex % cacheNumTilesX;
	uint32_t offsetY = light.shadowIndex / cacheNumTilesX;
	sf::Vec3 uvw = -volumeOrigin / volumeExtent;
	light.shadowBias = (uvw * sf::Vec3(1.0f, 1.0f, -1.0f) + sf::Vec3((float)offsetX, 0.0f, (float)offsetY + 1.0f)) / sf::Vec3((float)cacheNumTilesX, 1.0f, (float)cacheNumTilesY);
	light.shadowMul = sf::Vec3(-1.0f, -1.0f, 1.0f) / volumeExtent / sf::Vec3((float)cacheNumTilesX, 1.0f, (float)cacheNumTilesY);

	{
		sp::beginPass(shadowCachePass, NULL);

		gameShaders.shadowGridPipe.bind();

		ShadowGrid_Pixel_t pu;
		pu.depthSlices = (float)cacheTileSlices;
		pu.volumeOrigin = volumeOrigin;
		pu.volumeUnits = volumeExtent;
		pu.volumeUnits.y /= cacheTileSlices;
		sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_ShadowGrid_Pixel, &pu, sizeof(pu));

		sg_bindings bindings = { };
		bindings.vertex_buffers[0] = gameShaders.fullscreenTriangleBuffer;
		bindings.fs_images[SLOT_ShadowGrid_shadowDepth] = depthRenderCube.image;
		sg_apply_bindings(&bindings);

		sg_draw(0, 3, 1);

		sp::endPass();
	}

	{
		sf::SmallArray<sg_bqq_subimage_rect, 64> rects;
		for (uint32_t i = 0; i < cacheTileSlices; i++) {
			sg_bqq_subimage_rect &rect = rects.push();
			rect.src_x = (int)(i * cacheTileExtent);
			rect.src_y = 0;
			rect.dst_x = (int)(offsetX * cacheTileExtent);
			rect.dst_y = (int)(offsetY * cacheTileExtent);
			rect.dst_z = (int)i;
			rect.width = rect.height = (int)cacheTileExtent;
		}

		sg_bqq_subimage_copy_desc d = { };
		d.src_image = shadowCacheRender.image;
		d.dst_image = shadowCache.image;
		d.rects = rects.data;
		d.num_rects = rects.size;
		d.num_mips = 1;
		sg_bqq_copy_subimages(&d);
	}

}

}
