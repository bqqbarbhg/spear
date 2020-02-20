#include "sp/GameMain.h"

#include "sf/Sort.h"
#include "sf/String.h"

#include "sp/Canvas.h"
#include "sp/Sprite.h"

struct Game
{
	sp::SpriteRef sprite{"dude.png"};
	sp::Canvas canvas;

	Game()
	{
		sp::ContentFile::addRelativeFileRoot("data/");
	}

	void update(float dt)
	{
		canvas.clear();
		sp::SpriteDraw draw;
		draw.sprite = sprite;
		canvas.draw(draw);
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

		canvas.render(sp::CanvasRenderOpts{});

		sgl_draw();

		sg_end_pass();
		sg_commit();
	}
};

Game *game;

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
