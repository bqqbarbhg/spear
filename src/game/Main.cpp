#include "sp/GameMain.h"

#include "sf/Sort.h"
#include "sf/String.h"

#include "sp/Canvas.h"
#include "sp/Sprite.h"
#include "sp/Font.h"

static void appendUtf8(sf::StringBuf &buf, uint32_t code)
{
	if (code < 0x7f) {
		buf.append((char)code);
	} else if (code <= 0x7ff) {
		buf.append((char)(0xc0 | (code >> 6)));
		buf.append((char)(0x80 | (code >> 0 & 0x3f)));
	} else if (code <= 0xffff) {
		buf.append((char)(0xe0 | (code >> 12)));
		buf.append((char)(0x80 | (code >>  6 & 0x3f)));
		buf.append((char)(0x80 | (code >>  0 & 0x3f)));
	} else if (code <= 0x10ffff) {
		buf.append((char)(0xf0 | (code >> 18)));
		buf.append((char)(0x80 | (code >> 12 & 0x3f)));
		buf.append((char)(0x80 | (code >>  6 & 0x3f)));
		buf.append((char)(0x80 | (code >>  0 & 0x3f)));
	}
}

struct Game
{
	sp::Canvas canvas;
	sp::Canvas canvas2;
	sp::FontRef font{"sp://OpenSans-Ascii.ttf"};
	sp::FontRef jpFont{"data/kochi-mincho-subst.ttf"};
	uint32_t jpFrame = 0;

	Game()
	{
		sp::ContentFile::addRelativeFileRoot("");

		canvas.clear();

		sp::SpriteProps props;
		props.tileX = true;
		props.tileY = true;
		sp::SpriteRef grid{"data/hue.png", props};
		sp::SpriteRef badGrid{"data/hue.png"};
		sp::SpriteRef ghost{"data/ghost.png"};

		canvas.draw(grid, sf::Vec2(0.0f, 0.0f), sf::Vec2(200.0f, 200.0f), sf::Vec4(0.5f));
		canvas.draw(grid, sf::Vec2(200.0f, 0.0f), sf::Vec2(200.0f, 200.0f), sf::Vec4(0.5f));
		canvas.draw(badGrid, sf::Vec2(0.0f, 200.0f), sf::Vec2(200.0f, 200.0f), sf::Vec4(0.5f));
		canvas.draw(badGrid, sf::Vec2(200.0f, 200.0f), sf::Vec2(200.0f, 200.0f), sf::Vec4(0.5f));
		canvas.draw(ghost, sf::Vec2(400.0f, 100.0f), sf::Vec2(400.0f, 400.0f));

		sp::TextDraw td;
		td.font = font;
		td.string = "Typo, WOrld!";
		td.transform.m02 = 100.0f;
		td.transform.m12 = 500.0f;
		td.height = 60.0f;
		canvas.drawText(td);
	}

	void debugRenderAtlases()
	{
		sgl_defaults();

		sgl_enable_texture();

		sf::SmallArray<sg_image, 32> images;

		images.push(sp::Font::getFontAtlasImage());

		{
			sf::SmallArray<sp::Atlas*, 32> atlases;
			sp::Atlas::getAtlases(atlases);
			for (sp::Atlas *atlas : atlases) {
				images.push(atlas->image);
			}
		}

		float xs = 0.2f * (float)sapp_height() / (float)sapp_width();
		float ys = -0.2f;
		float y = -1.0f - ys;
		float x = -1.0f;
		int n = 0;
		for (sg_image image : images) {
			sgl_texture(image);

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
		}	}

	void update(float dt)
	{
		debugRenderAtlases();

		if (jpFont->isLoaded()) {
			jpFrame++;
		}

		canvas2.clear();
		sf::StringBuf str;
		for (uint32_t n = 0; n < 32; n++) {
			appendUtf8(str, 0x4e00 + (jpFrame + n) % 100);
		}
		sp::TextDraw td;
		td.font = jpFont;
		td.string = str;
		td.transform.m02 = 100.0f;
		td.transform.m12 = 700.0f;
		td.height = 200.0f;
		canvas2.drawText(td);

	}

	void render()
	{
		sg_pass_action pass_action = { };
		pass_action.colors[0].action = SG_ACTION_CLEAR;
#if 1
		pass_action.colors[0].val[0] = (float)0x64 / 255.0f;
		pass_action.colors[0].val[1] = (float)0x95 / 255.0f;
		pass_action.colors[0].val[2] = (float)0xed / 255.0f;
		pass_action.colors[0].val[3] = (float)0xff / 255.0f;
#else
		pass_action.colors[0].val[0] = 0.0f;
		pass_action.colors[0].val[1] = 0.0f;
		pass_action.colors[0].val[2] = 0.0f;
		pass_action.colors[0].val[3] = 0.0f;
#endif

		sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());

		canvas.render(sp::CanvasRenderOpts::windowPixels());
		canvas2.render(sp::CanvasRenderOpts::windowPixels());

		sp::CanvasRenderOpts opts = sp::CanvasRenderOpts::windowPixels();
		opts.transform = sf::mat::scale(0.2f) * opts.transform;

		canvas2.render(opts);

		sgl_draw();

		sg_end_pass();
		sg_commit();
	}
};

Game *game;

void spConfig(sp::MainConfig &config)
{
	config.sappDesc->window_title = "Spear";
	config.sappDesc->sample_count = 4;
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
