#include "LightSystem.h"

#include "client/AreaSystem.h"

#include "sp/Renderer.h"

#include "game/shader/GameShaders.h"
#include "game/shader/ShadowGrid.h"

#include "game/DebugDraw.h"

namespace cl {

struct ShadowCache
{
	uint32_t cacheTileExtent = 128;
	uint32_t cacheTileSlices = 8;
	uint32_t cacheNumTilesX = 8;
	uint32_t cacheNumTilesY = 8;

	uint32_t depthRenderExtent = 512;

	sp::Texture shadowTexture;

	sp::RenderTarget shadowCacheRender;
	sp::RenderPass shadowCachePass;

	sp::Texture depthRenderCube;
	sp::RenderTarget depthRenderDepth;
	sp::RenderPass depthRenderPass[6];

	void recreateTargets()
	{
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
			shadowTexture.init(d);
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
};

struct LightSystemImp final : LightSystem
{
	struct PointLightImp
	{
		uint32_t areaId = ~0u;
		uint32_t entityId;

		uint32_t fadeIndex = ~0u;

		sf::Vec3 offset;

		sf::Vec3 baseColor;

		// Should match PointLight for faster copying
		sf::Sphere sphere;
		sf::Vec3 currentColor;
		sf::Vec3 shadowMul;
		sf::Vec3 shadowBias;

		float fadeValue = 0.0f;
		float fadeInSpeed = 0.0f;
		float fadeOutSpeed = 0.0f;

		bool isFadingOut = false;

		bool hasShadows = false;
		uint64_t shadowRenderFrame;
	};

	ShadowCache shadowCache;

	sf::Array<PointLightImp> pointLights;
	sf::Array<uint32_t> freePointLightIds;
	sf::Array<uint32_t> fadingPointLightIds;

	double gameTime = 0.0;

	void renderPointLightShadows(Systems &systems, uint32_t pointId)
	{
		PointLightImp &point = pointLights[pointId];

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
		action.colors[0].val[0] = point.sphere.radius;
		action.colors[0].val[1] = point.sphere.radius;
		action.colors[0].val[2] = point.sphere.radius;
		action.colors[0].val[3] = point.sphere.radius;
		action.depth.action = SG_ACTION_CLEAR;
		action.depth.val = 1.0f;

		float clipNearW = sg_query_features().origin_top_left ? -1.0f : 0.0f;
		for (uint32_t side = 0; side < 6; side++) {
			RenderArgs args;

			const sf::Vec3 *basis = cubeBasis[side];
			sf::Mat34 view = sf::mat::look(point.sphere.origin, basis[0], basis[1]);
			sf::Mat44 proj = sf::mat::perspectiveD3D(sf::F_PI/2.0f, 1.0f, 0.1f, point.sphere.radius);
			args.worldToClip = proj * view;
			args.cameraPosition = point.sphere.origin;
			args.frustum = sf::Frustum(args.worldToClip, clipNearW);

			sp::beginPass(shadowCache.depthRenderPass[side], &action);
			systems.renderShadows(args);
			sp::endPass();
		}

		float radius = point.sphere.radius;
		float height = 4.0f;

		uint32_t cacheTileExtent = shadowCache.cacheTileExtent;
		uint32_t cacheNumTilesX = shadowCache.cacheNumTilesX;
		uint32_t cacheNumTilesY = shadowCache.cacheNumTilesY;

		sf::Vec3 f = sf::Vec3((float)cacheTileExtent, (float)shadowCache.cacheTileSlices, (float)cacheTileExtent) / (point.sphere.radius*2.0f);
		// f = sf::Vec3(1.0f);
		sf::Vec3 p = point.sphere.origin * f;
		p.x = p.x - floorf(p.x);
		// p.y = p.y - floorf(p.y);
		p.z = p.z - floorf(p.z);
		sf::Vec3 volumeOrigin = sf::Vec3(-radius, -1.0f, -radius) - p / f;
		sf::Vec3 volumeExtent = sf::Vec3(radius*2.0f, height, radius*2.0f);

		uint32_t shadowIndex = pointId;
		uint32_t offsetX = shadowIndex % shadowCache.cacheNumTilesX;
		uint32_t offsetY = shadowIndex / shadowCache.cacheNumTilesX;
		sf::Vec3 uvw = -volumeOrigin / volumeExtent;
		point.shadowBias = (uvw * sf::Vec3(1.0f, 1.0f, -1.0f) + sf::Vec3((float)offsetX, 0.0f, (float)offsetY + 1.0f)) / sf::Vec3((float)cacheNumTilesX, 1.0f, (float)cacheNumTilesY);
		point.shadowMul = sf::Vec3(-1.0f, -1.0f, 1.0f) / volumeExtent / sf::Vec3((float)cacheNumTilesX, 1.0f, (float)cacheNumTilesY);

		{
			sp::beginPass(shadowCache.shadowCachePass, NULL);

			gameShaders.shadowGridPipe.bind();

			ShadowGrid_Pixel_t pu;
			pu.depthSlices = (float)shadowCache.cacheTileSlices;
			pu.volumeOrigin = volumeOrigin;
			pu.volumeUnits = volumeExtent;
			pu.volumeUnits.y /= shadowCache.cacheTileSlices;
			sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_ShadowGrid_Pixel, &pu, sizeof(pu));

			sg_bindings bindings = { };
			bindings.vertex_buffers[0] = gameShaders.fullscreenTriangleBuffer;
			bindings.fs_images[SLOT_ShadowGrid_shadowDepth] = shadowCache.depthRenderCube.image;
			sg_apply_bindings(&bindings);

			sg_draw(0, 3, 1);

			sp::endPass();
		}

		{
			sf::SmallArray<sg_bqq_subimage_rect, 64> rects;
			for (uint32_t i = 0; i < shadowCache.cacheTileSlices; i++) {
				sg_bqq_subimage_rect &rect = rects.push();
				rect.src_x = (int)(i * cacheTileExtent);
				rect.src_y = 0;
				rect.dst_x = (int)(offsetX * cacheTileExtent);
				rect.dst_y = (int)(offsetY * cacheTileExtent);
				rect.dst_z = (int)i;
				rect.width = rect.height = (int)cacheTileExtent;
			}

			sg_bqq_subimage_copy_desc d = { };
			d.src_image = shadowCache.shadowCacheRender.image;
			d.dst_image = shadowCache.shadowTexture.image;
			d.rects = rects.data;
			d.num_rects = rects.size;
			d.num_mips = 1;
			sg_bqq_copy_subimages(&d);
		}
	}

	// API

	LightSystemImp()
	{
		shadowCache.recreateTargets();
	}

	void addPointLight(Systems &systems, uint32_t entityId, uint8_t componentIndex, const sv::PointLightComponent &c, const Transform &transform) override
	{
		uint32_t pointId = pointLights.size;
		if (freePointLightIds.size > 0) {
			pointId = freePointLightIds.popValue();
		} else {
			pointLights.push();
		}

		PointLightImp &point = pointLights[pointId];
		point.entityId = entityId;

		point.offset = c.position;
		point.sphere.radius = c.radius;
		point.sphere.origin = sf::transformPoint(transform.asMatrix(), point.offset);
		point.fadeInSpeed = c.fadeInTime > 0.001f ? 1.0f / c.fadeInTime : 0.0f;
		point.fadeOutSpeed = c.fadeOutTime > 0.001f ? 1.0f / c.fadeOutTime : 0.0f;
		point.baseColor = c.color * c.intensity;
		point.hasShadows = c.hasShadows;

		point.areaId = systems.area->addSphereArea(AreaGroup::PointLight, pointId, point.sphere, Area::Visibility);

		if (point.fadeInSpeed > 0.0f) {
			point.fadeIndex = fadingPointLightIds.size;
			fadingPointLightIds.push(pointId);
		} else {
			point.fadeValue = 1.0f;
			point.currentColor = point.baseColor;
		}

		uint32_t entityFlags = Entity::UpdateTransform | Entity::PrepareForRemove;
		systems.entities.addComponent(entityId, this, pointId, 0, componentIndex, entityFlags);
	}

	void updateTransform(Systems &systems, uint32_t entityId, const EntityComponent &ec, const TransformUpdate &update) override
	{
		uint32_t pointId = ec.userId;
		PointLightImp &point = pointLights[pointId];

		point.sphere.origin = sf::transformPoint(update.entityToWorld, point.offset);

		systems.area->updateSphereArea(point.areaId, point.sphere);
	}

	bool prepareForRemove(Systems &systems, uint32_t entityId, const EntityComponent &ec, const FrameArgs &args) override
	{
		uint32_t pointId = ec.userId;
		PointLightImp &point = pointLights[pointId];
		if (point.fadeOutSpeed <= 0.0f) return true;

		if (!point.isFadingOut) {
			point.isFadingOut = true;
			if (point.fadeIndex == ~0u) {
				point.fadeIndex = fadingPointLightIds.size;
				fadingPointLightIds.push(pointId);
			}
		}

		return point.fadeValue <= 0.0f;
	}

	void remove(Systems &systems, uint32_t entityId, const EntityComponent &ec) override
	{
		uint32_t pointId = ec.userId;
		PointLightImp &point = pointLights[pointId];

		if (point.areaId != ~0u) {
			systems.area->removeSphereArea(point.areaId);
		}

		if (point.fadeIndex != ~0u) {
			pointLights[fadingPointLightIds.back()].fadeIndex = point.fadeIndex;
			fadingPointLightIds.removeSwap(point.fadeIndex);
		}

		freePointLightIds.push(pointId);
		sf::reset(point);
	}

	void updateLightFade(const FrameArgs &frameArgs) override
	{
		float dt = frameArgs.dt;
		for (uint32_t i = 0; i < fadingPointLightIds.size; i++) {
			uint32_t pointId = fadingPointLightIds[i];
			PointLightImp &point = pointLights[pointId];
			bool remove = false;
			if (point.isFadingOut) {
				point.fadeValue -= point.fadeOutSpeed * dt;
				if (point.fadeValue < 0.0f) {
					point.fadeValue = 0.0f;
					remove = true;
				}
			} else {
				point.fadeValue += point.fadeInSpeed * dt;
				if (point.fadeValue > 1.0f) {
					point.fadeValue = 1.0f;
					remove = true;
				}
			}

			point.currentColor = point.baseColor * point.fadeValue;

			if (remove) {
				pointLights[fadingPointLightIds.back()].fadeIndex = i;
				point.fadeIndex = ~0u;
				fadingPointLightIds.removeSwap(i--);
			}
		}
	}

	void queryVisiblePointLights(const VisibleAreas &visibleAreas, sf::Array<PointLight> &outPointLights, const sf::Bounds3 &bounds) const override
	{
		for (uint32_t pointId : visibleAreas.get(AreaGroup::PointLight)) {
			const PointLightImp &point = pointLights[pointId];
			if (sf::intersect(bounds, point.sphere)) {
				PointLight &outPoint = outPointLights.push();
				outPoint.position = point.sphere.origin;
				outPoint.radius = point.sphere.radius;
				outPoint.color = point.currentColor;
				outPoint.shadowMul = point.shadowMul;
				outPoint.shadowBias = point.shadowBias;
			}
		}
	}

	void renderShadowMaps(Systems &systems, const VisibleAreas &visibleAreas) override
	{
		for (uint32_t pointId : visibleAreas.get(AreaGroup::PointLight)) {
			PointLightImp &point = pointLights[pointId];
			if (!point.hasShadows) continue;

			renderPointLightShadows(systems, pointId);
			point.shadowRenderFrame = systems.frameArgs.frameIndex;
		}
	}

	virtual sg_image getShadowTexture() const override
	{
		return shadowCache.shadowTexture.image;
	}

	void editorHighlight(Systems &systems, const EntityComponent &ec, EditorHighlight type) override
	{
		if (type != EditorHighlight::Select) return;

		if (ec.subsystemIndex == 0) {
			uint32_t pointId = ec.userId;
			const PointLightImp &point = pointLights[pointId];

			debugDrawSphere(point.sphere.origin, 0.1f);
		}
	}

};

sf::Box<LightSystem> LightSystem::create() { return sf::box<LightSystemImp>(); }

}
