#include "LightState.h"

#include "client/ClientGlobal.h"
#include "client/ClientState.h"
#include "sp/Renderer.h"

#include "game/shader/GameShaders.h"
#include "game/shader/ShadowGrid.h"

namespace cl {

struct PointLightImp
{
	uint32_t entityId;
	sf::Vec3 localPosition;
	sf::Vec3 worldPosition;
	sf::Vec3 color;
	float radius;
	sf::Vec3 shadowMul;
	sf::Vec3 shadowBias;
	uint32_t shadowIndex = ~0u;
	uint32_t visibleIndex = ~0u;
	bool castShadows;
	bool dirtyShadows = true;
};

struct LightEntityImp
{
	sf::SmallArray<uint32_t, 2> pointLights;
};

struct FreeShadowIndex
{
	uint32_t index;
	uint64_t frameIndex;
};

struct LightStateImp : LightState
{
	sf::HashMap<uint32_t, LightEntityImp> entities;

	// -- Point lights
	sf::Array<uint32_t> visiblePointLights;
	sf::Array<PointLightImp> pointLights;
	sf::Array<uint32_t> freePointLightIndices;
	sf::Array<uint32_t> dirtyShadowPointLights;
	sf::Array<FreeShadowIndex> freeShadowIndices;

	// -- Shadow cache
	sp::Texture shadowCacheTexture;
	uint32_t cacheTileExtent;
	uint32_t depthRenderExtent;
	uint32_t cacheTileSlices = 8;
	uint32_t cacheNumTilesX = 8;
	uint32_t cacheNumTilesY = 8;
	sp::RenderTarget shadowCacheRender;
	sp::RenderPass shadowCachePass;
	sp::Texture depthRenderCube;
	sp::RenderTarget depthRenderDepth;
	sp::RenderPass depthRenderPass[6];

	LightStateImp()
	{
		cacheTileExtent = clientGlobal->settings.shadowResolution;
		depthRenderExtent = clientGlobal->settings.shadowRenderResolution;

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
			shadowCacheTexture.init(d);
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

		for (uint32_t i = 0; i < cacheNumTilesX * cacheNumTilesY; i++) {
			freeShadowIndices.push({ i, UINT64_MAX });
		}
	}

	uint32_t allocateShadowIndex()
	{
		if (freeShadowIndices.size == 0) return;
		FreeShadowIndex *max = &freeShadowIndices[0];
		for (FreeShadowIndex &index : freeShadowIndices.slice().drop(1)) {
			if (max->frameIndex == UINT64_MAX) break;
			if (index.frameIndex > max->frameIndex) {
				max = &index;
			}
		}
		uint32_t index = max->index;
		freeShadowIndices.removeSwapPtr(max);
		return index;
	}

	void addPointLight(uint32_t entityId, const sv::PointLightComponent &c)
	{
		uint32_t index = pointLights.size;
		if (freePointLightIndices.size > 0) {
			index = freePointLightIndices.popValue();
		} else {
			pointLights.push();
		}
		PointLightImp &point = pointLights[index];
		point.localPosition = c.position;
		point.radius = c.radius;
		point.color = c.color * c.intensity;
		point.castShadows = c.castShadows;

		LightEntityImp& entity = entities[entityId];
		entity.pointLights.push(index);
	}

	void updateEntity(uint32_t entityId, const EntityState &state, uint32_t updateMask)
	{
		LightEntityImp& entity = *entities.findValue(entityId);

		for (uint32_t pointIndex : entity.pointLights) {
			PointLightImp &point = pointLights[pointIndex];

			if (updateMask & EntityState::UpdateTransform) {
				point.worldPosition = sf::transformPoint(state.transform, point.localPosition);
				if (point.shadowIndex != ~0u && !point.dirtyShadows) {
					dirtyShadowPointLights.push(pointIndex);
					point.dirtyShadows = true;
				}
			}

			if (updateMask & EntityState::UpdateVisibility) {
				if (state.flags & EntityState::Visible) {
					if (point.visibleIndex == ~0u) {
						point.visibleIndex = visiblePointLights.size;
						visiblePointLights.push(pointIndex);
					}
					if (point.shadowIndex == ~0u) {
						point.shadowIndex = allocateShadowIndex();
						if (point.shadowIndex != ~0u && !point.dirtyShadows) {
							dirtyShadowPointLights.push(pointIndex);
							point.dirtyShadows = true;
						}
					}
				} else {
					if (point.visibleIndex != ~0u) {
						visiblePointLights.removeSwap(point.visibleIndex);
						if (point.visibleIndex < visiblePointLights.size) {
							PointLightImp &swapPoint = pointLights[visiblePointLights[point.visibleIndex]];
							swapPoint.visibleIndex = point.visibleIndex;
						}
						point.visibleIndex = ~0u;
					}
					if (point.shadowIndex != ~0u) {
						freeShadowIndices.push({ point.shadowIndex, clientGlobal->frameIndex });
						point.shadowIndex = ~0u;
					}
				}
			}
		}
	}

	void removeEntity(uint32_t entityId)
	{
		LightEntityImp& entity = *entities.findValue(entityId);

		for (uint32_t pointIndex : entity.pointLights) {
			PointLightImp &point = pointLights[pointIndex];
			if (point.visibleIndex != ~0u) {
				visiblePointLights.removeSwap(point.visibleIndex);
				if (point.visibleIndex < visiblePointLights.size) {
					PointLightImp &swapPoint = pointLights[visiblePointLights[point.visibleIndex]];
					swapPoint.visibleIndex = point.visibleIndex;
				}
			}
			if (point.shadowIndex != ~0u) {
				freeShadowIndices.push({ point.shadowIndex, UINT64_MAX });
			}
			if (point.dirtyShadows) {
				sf::findRemoveSwap(dirtyShadowPointLights, pointIndex);
			}
			freePointLightIndices.push(pointIndex);
		}

		entities.remove(entityId);
	}

	void renderPointLightShadow(uint32_t pointIndex)
	{
		PointLightImp &point = pointLights[pointIndex];
		if (point.shadowIndex == ~0u) return;

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
		action.colors[0].val[0] = point.radius;
		action.colors[0].val[1] = point.radius;
		action.colors[0].val[2] = point.radius;
		action.colors[0].val[3] = point.radius;
		action.depth.action = SG_ACTION_CLEAR;
		action.depth.val = 1.0f;

		float clipNearW = sg_query_features().origin_top_left ? -1.0f : 0.0f;
		for (uint32_t side = 0; side < 6; side++) {
			RenderShadowArgs args;

			const sf::Vec3 *basis = cubeBasis[side];
			sf::Mat34 view = sf::mat::look(point.worldPosition, basis[0], basis[1]);
			sf::Mat44 proj = sf::mat::perspectiveD3D(sf::F_PI/2.0f, 1.0f, 0.1f, point.radius);
			args.worldToClip = proj * view;
			args.cameraPosition = point.worldPosition;
			args.frustum = sf::Frustum(args.worldToClip, clipNearW);

			sp::beginPass(depthRenderPass[side], &action);
			clientGlobal->clientState->renderShadows(args);
			sp::endPass();
		}

		float radius = point.radius;
		float height = 4.0f;

		sf::Vec3 f = sf::Vec3((float)cacheTileExtent, (float)cacheTileSlices, (float)cacheTileExtent) / (point.radius*2.0f);
		// f = sf::Vec3(1.0f);
		sf::Vec3 p = point.worldPosition * f;
		p.x = p.x - floorf(p.x);
		// p.y = p.y - floorf(p.y);
		p.z = p.z - floorf(p.z);
		sf::Vec3 volumeOrigin = sf::Vec3(-radius, -1.0f, -radius) - p / f;
		sf::Vec3 volumeExtent = sf::Vec3(radius*2.0f, height, radius*2.0f);

		uint32_t offsetX = point.shadowIndex % cacheNumTilesX;
		uint32_t offsetY = point.shadowIndex / cacheNumTilesX;
		sf::Vec3 uvw = -volumeOrigin / volumeExtent;
		point.shadowBias = (uvw * sf::Vec3(1.0f, 1.0f, -1.0f) + sf::Vec3((float)offsetX, 0.0f, (float)offsetY + 1.0f)) / sf::Vec3((float)cacheNumTilesX, 1.0f, (float)cacheNumTilesY);
		point.shadowMul = sf::Vec3(-1.0f, -1.0f, 1.0f) / volumeExtent / sf::Vec3((float)cacheNumTilesX, 1.0f, (float)cacheNumTilesY);

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
			d.dst_image = shadowCacheTexture.image;
			d.rects = rects.data;
			d.num_rects = rects.size;
			d.num_mips = 1;
			sg_bqq_copy_subimages(&d);
		}
	}

	void renderDirtyShadowMaps(uint32_t maxUpdates)
	{
		for (uint32_t i = 0; i < maxUpdates; i++) {
			uint32_t pointIndex = dirtyShadowPointLights[i];
			pointLights[pointIndex].dirtyShadows = false;
			renderPointLightShadow(pointIndex);
			maxUpdates--;
		}
		if (maxUpdates < dirtyShadowPointLights.size) {
			dirtyShadowPointLights.removeOrdered(0, maxUpdates);
		} else {
			dirtyShadowPointLights.clear();
		}
	}

	void queryVisiblePointLights(const sf::Bounds3 &bounds, uint32_t maxPointLights, sf::Array<PointLight> &outPointLights)
	{
		for (uint32_t pointIndex : visiblePointLights) {
			PointLightImp &point = pointLights[pointIndex];
			sf::Sphere sphere = { point.worldPosition, point.radius };
			if (sf::intersect(bounds, sphere)) {
				if (outPointLights.size < maxPointLights) {
					PointLight &out = outPointLights.push();
					out.position = point.worldPosition;
					out.color = point.color;
					out.radius = point.radius;
					out.shadowBias = point.shadowBias;
					out.shadowMul = point.shadowMul;
				}
			}
		}
	}

};

sf::Box<LightState> LightState::create()
{
	return sf::box<LightStateImp>();
}

void LightState::addPointLight(uint32_t entityId, const sv::PointLightComponent &c)
{
	((LightStateImp*)this)->addPointLight(entityId, c);
}

void LightState::updateEntity(uint32_t entityId, const EntityState &state, uint32_t updateMask)
{
	((LightStateImp*)this)->updateEntity(entityId, state, updateMask);
}

void LightState::removeEntity(uint32_t entityId)
{
	((LightStateImp*)this)->removeEntity(entityId);
}

void LightState::renderDirtyShadowMaps(uint32_t maxUpdates)
{
	((LightStateImp*)this)->renderDirtyShadowMaps(maxUpdates);
}

void LightState::queryVisiblePointLights(const sf::Bounds3 &bounds, uint32_t maxPointLights, sf::Array<PointLight> &pointLights)
{
	((LightStateImp*)this)->queryVisiblePointLights(bounds, maxPointLights, pointLights);
}

sg_image LightState::getShadowCache() const
{
	return ((LightStateImp*)this)->shadowCacheTexture.image;
}

}
