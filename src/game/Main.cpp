#include "sp/GameMain.h"

#include "sf/Sort.h"

int LOOPCOUNT;

struct Game
{
	Game()
	{
	}

	void update(float dt)
	{
	}

	void render()
	{
	}
};

Game *game;

void spInit()
{
	sf::Array<int> nums;

	uint32_t rng = 1;

	nums.push(1);
	for (int i = 0; i < 200000; i++) {
		nums.push(0);
	}
	nums.push(1);

	sf::sort(nums);

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
