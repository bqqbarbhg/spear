#include "ext/sokol/sokol_app.h"
#include "ext/sokol/sokol_gfx.h"
#include "ext/sokol/sokol_gl.h"
#include "ext/sokol/sokol_imgui.h"
#include "ext/sokol/sokol_time.h"

#include "sf/Base.h"

#include "sp/Sprite.h"
#include "sp/Asset.h"
#include "sp/ContentFile.h"
#include "sp/Canvas.h"

constexpr uint32_t MsaaSamples = 1;

struct TestGame
{
	sp::SpriteRef dude { "dude.png" };
	sp::SpriteRef guy { "guy.png" };

	sf::Array<sp::SpriteRef> items;
	sp::Canvas canvas;

	uint32_t frameIndex = 0;

	TestGame()
	{
		items.push(sp::SpriteRef{"Card/Axe.png" });
		items.push(sp::SpriteRef{"Card/Bow.png" });
		items.push(sp::SpriteRef{"Card/Club.png" });
		items.push(sp::SpriteRef{"Card/Dagger.png" });
		items.push(sp::SpriteRef{"Card/Firebolt_Spell.png" });
		items.push(sp::SpriteRef{"Card/Flame_Spell.png" });
		items.push(sp::SpriteRef{"Card/Hammer.png" });
		items.push(sp::SpriteRef{"Card/Healing_Potion.png" });
		items.push(sp::SpriteRef{"Card/Light_Helmet.png" });
		items.push(sp::SpriteRef{"Card/Longsword.png" });
		items.push(sp::SpriteRef{"Card/Shortsword.png" });
		items.push(sp::SpriteRef{"Card/Spear.png" });
		items.push(sp::SpriteRef{"Card/Vest.png" });
	}

	void render()
	{
		sp::ContentFile::update();
		sp::Asset::update();
		sp::Sprite::update();

		canvas.clear();

		canvas.draw(dude, sf::Mat23());
		canvas.draw(guy, sf::Mat23());

		for (sp::SpriteRef &ref : items) {
			canvas.draw(ref, sf::Mat23());
		}

		sgl_defaults();

		sgl_enable_texture();

		sf::SmallArray<sp::Atlas*, 8> atlases;
		sp::Atlas::getAtlases(atlases);

		float y = 1.0f;
		float x = -1.0f;
		float xs = 0.2f;
		float ys = -0.2f;
		int n = 0;
		for (sp::Atlas *atlas : atlases) {
			sg_image image;
			image.id = atlas->getTexture();
			sgl_texture(image);

			sgl_begin_quads();
			sgl_v2f_t2f(x, y, 0.0f, 0.0f);
			sgl_v2f_t2f(x+xs, y, 1.0f, 0.0f);
			sgl_v2f_t2f(x+xs, y+ys, 1.0f, 1.0f);
			sgl_v2f_t2f(x, y+ys, 0.0f, 1.0f);
			sgl_end();

			if (++n % 2 == 0) {
				x += xs;
				y = 1.0f;
			} else {
				y += ys;
			}
		}

		sp::Sprite::updateAtlasesForRendering();

		sg_pass_action pass_action = { };
		pass_action.colors[0].action = SG_ACTION_CLEAR;
		pass_action.colors[0].val[0] = (float)0x64 / 255.0f;
		pass_action.colors[0].val[1] = (float)0x95 / 255.0f;
		pass_action.colors[0].val[2] = (float)0xed / 255.0f;
		pass_action.colors[0].val[3] = (float)0xff / 255.0f;
		sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());

		sgl_draw();

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

	sp::ContentFile::init("data/");
	sp::Asset::init();

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
