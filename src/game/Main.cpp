#include "sp/GameMain.h"

#include "sf/Sort.h"
#include "sf/String.h"

#include "sp/Canvas.h"
#include "sp/Sprite.h"

struct Game
{
	sp::Canvas canvas;

	Game()
	{
		sp::ContentFile::addRelativeFileRoot("");

		canvas.clear();

		for (int y = 0; y < 5; y++) {
			for (int x = 0; x < 10; x++) {
				sf::StringBuf url;
				url.format("https://placekitten.com/%d/%d", 100 + x, 100 + y);
				sp::SpriteRef sprite{url};
				sp::SpriteDraw draw;
				draw.sprite = sprite;
				draw.transform = sf::mat2D::scale(200.0f, 200.0f) * sf::mat2D::translate((float)x, (float)y);
				canvas.draw(draw);
			}
		}
	}

	void debugRenderAtlases()
	{
		sgl_defaults();

		sgl_enable_texture();

		sf::SmallArray<sp::Atlas*, 8> atlases;
		sp::Atlas::getAtlases(atlases);

		float xs = 0.2f * (float)sapp_height() / (float)sapp_width();
		float ys = -0.2f;
		float y = -1.0f - ys;
		float x = -1.0f;
		int n = 0;
		for (sp::Atlas *atlas : atlases) {
			sgl_texture(atlas->image);

			sgl_begin_quads();
			sgl_v2f_t2f(x, y, 0.0f, 0.0f);
			sgl_v2f_t2f(x+xs, y, 1.0f, 0.0f);
			sgl_v2f_t2f(x+xs, y+ys, 1.0f, 1.0f);
			sgl_v2f_t2f(x, y+ys, 0.0f, 1.0f);
			sgl_end();

			if (++n % 2 == 0) {
				x += xs;
				y = -1.0f - ys;
			} else {
				y -= ys;
			}
		}
	}

	void update(float dt)
	{
		debugRenderAtlases();
	}

	void render()
	{
		sg_pass_action pass_action = { };
		pass_action.colors[0].action = SG_ACTION_CLEAR;
		pass_action.colors[0].val[0] = (float)0x64 / 255.0f;
		pass_action.colors[0].val[1] = (float)0x95 / 255.0f;
		pass_action.colors[0].val[2] = (float)0xed / 255.0f;
		pass_action.colors[0].val[3] = (float)0xff / 255.0f;

		sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());

		canvas.render(sp::CanvasRenderOpts::windowPixels());

		sgl_draw();

		sg_end_pass();
		sg_commit();
	}
};

Game *game;

void spConfig(sp::MainConfig &config)
{
	config.sappDesc->window_title = "Spear";
}

void spInit()
{
	game = new Game();
}

void spCleanup()
{
	delete game;
}

void spEvent(const sapp_event *e)
{
}

void spFrame(float dt)
{
	game->update(dt);
	game->render();
}
