#pragma once

#ifndef SP_WINDOW_WIDTH
#define SP_WINDOW_WIDTH 1400
#endif

#ifndef SP_WINDOW_HEIGHT
#define SP_WINDOW_HEIGHT 1000
#endif

#ifndef SP_WINDOW_TITLE
#define SP_WINDOW_TITLE "SP Game"
#endif

// Include this in only one file!
// Sets up a main loop:
// spInit()
// spCleanup()
// spFrame()

#include "ext/sokol/sokol_app.h"
#include "ext/sokol/sokol_gfx.h"
#include "ext/sokol/sokol_gl.h"
#include "ext/sokol/sokol_time.h"

#include "Asset.h"
#include "Sprite.h"
#include "Font.h"
#include "ContentFile.h"
#include "Canvas.h"

namespace sp {

struct MainConfig
{
	sg_desc sgDesc = { };
	sapp_desc sappDesc = { };
	bool useContentThread = true;
};

}

void spConfig(sp::MainConfig &config);
void spInit();
void spCleanup();
void spEvent(const sapp_event *e);
void spFrame(float dt);

namespace sp {

static uint64_t impLastTime;
static int impSampleCount;
static MainConfig impConfig;

void impInit()
{
	sp::MainConfig &config = sp::impConfig;

	{
		sg_desc &desc = config.sgDesc;
		desc.gl_force_gles2 = sapp_gles2();
		desc.mtl_device = sapp_metal_get_device();
		desc.mtl_renderpass_descriptor_cb = sapp_metal_get_renderpass_descriptor;
		desc.mtl_drawable_cb = sapp_metal_get_drawable;
		desc.d3d11_device = sapp_d3d11_get_device();
		desc.d3d11_device_context = sapp_d3d11_get_device_context();
		desc.d3d11_render_target_view_cb = sapp_d3d11_get_render_target_view;
		desc.d3d11_depth_stencil_view_cb = sapp_d3d11_get_depth_stencil_view;


		sg_setup(&desc);
	}

	{
		sgl_desc_t desc = { };
		desc.sample_count = impSampleCount;
		sgl_setup(&desc);
	}

	stm_setup();

	sp::ContentFile::globalInit(config.useContentThread);
	sp::Asset::globalInit();
	sp::Sprite::globalInit();
	sp::Canvas::globalInit();
	sp::Font::globalInit();

	spInit();
}

void impCleanup()
{
	spCleanup();

	sp::Canvas::globalCleanup();
	sp::Sprite::globalCleanup();
	sp::Font::globalCleanup();
	sp::Asset::globalCleanup();
	sp::ContentFile::globalCleanup();
}

void impEvent(const sapp_event *e)
{
	spEvent(e);
}

void impFrame()
{
	sp::ContentFile::globalUpdate();
	sp::Asset::globalUpdate();
	sp::Sprite::globalUpdate();
	sp::Font::globalUpdate();
	sp::Canvas::globalUpdate();

	float dt = (float)stm_sec(stm_laptime(&impLastTime));
	spFrame(dt);
}

}

sapp_desc sokol_main(int argc, char **argv)
{
	sp::MainConfig &config = sp::impConfig;

	config.sappDesc.init_cb = &sp::impInit;
	config.sappDesc.event_cb = &sp::impEvent;
	config.sappDesc.frame_cb = &sp::impFrame;
	config.sappDesc.cleanup_cb = &sp::impCleanup;

	config.sappDesc.width = SP_WINDOW_WIDTH;
	config.sappDesc.height = SP_WINDOW_HEIGHT;
	config.sappDesc.window_title = SP_WINDOW_TITLE;
	config.sappDesc.sample_count = 1;

	config.sappDesc.high_dpi = true;

	spConfig(config);

	sp::impSampleCount = config.sappDesc.sample_count;

	return config.sappDesc;
}
