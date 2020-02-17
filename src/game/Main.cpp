#include "ext/sokol/sokol_app.h"
#include "ext/sokol/sokol_gfx.h"
#include "ext/sokol/sokol_gl.h"
#include "ext/sokol/sokol_imgui.h"
#include "ext/sokol/sokol_time.h"

#include "sf/Base.h"

#include "sp/Sprite.h"

constexpr uint32_t MsaaSamples = 1;

struct TestGame
{
	sp::SpriteRef sprite { "dude.png" };

	void render()
	{
		sg_pass_action pass_action = { };
		pass_action.colors[0].action = SG_ACTION_CLEAR;
		pass_action.colors[0].val[0] = (float)0x64 / 255.0f;
		pass_action.colors[0].val[1] = (float)0x95 / 255.0f;
		pass_action.colors[0].val[2] = (float)0xed / 255.0f;
		pass_action.colors[0].val[3] = (float)0xff / 255.0f;
		sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());

		sprite->shouldBeLoaded();

		sg_end_pass();
		sg_commit();
	}
};

TestGame *game;

uint64_t g_last_time;

static void init()
{
	{
		sg_desc desc = { };

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
		desc.sample_count = MsaaSamples;
		sgl_setup(&desc);
	}

	stm_setup();

	game = new TestGame();
}

static void event(const sapp_event *e)
{
}

static void frame()
{
	float dt = (float)stm_sec(stm_laptime(&g_last_time));

	game->render();
}

static void cleanup()
{
}

sapp_desc sokol_main(int argc, char **argv)
{
	sapp_desc desc = { };

	desc.init_cb = &init;
	desc.event_cb = &event;
	desc.frame_cb = &frame;
	desc.cleanup_cb = &cleanup;

	desc.width = 1400;
	desc.height = 1000;
	desc.window_title = "Spear";

	desc.sample_count = MsaaSamples;

	return desc;
}
