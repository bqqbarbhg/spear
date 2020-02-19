#include "sp/GameMain.h"

#include "sf/Sort.h"

#include <algorithm>

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

	nums.clear();
	for (int i = 0; i < 2000000; i++) {
		rng ^= rng << 13;
		rng ^= rng >> 17;
		rng ^= rng << 5;
		nums.push(0);
	}

	uint64_t begin, end;

	begin = stm_now();
	sf::sort(nums);
	end = stm_now();
	sf::debugPrintLine(" sf: %.2fms", stm_ms(end - begin));

	for (size_t i = 1; i < nums.size; i++) {
		sf_assert(nums[i - 1] <= num[i]);
	}

	rng = 1;

	nums.clear();
	for (int i = 0; i < 2000000; i++) {
		rng ^= rng << 13;
		rng ^= rng >> 17;
		rng ^= rng << 5;
		nums.push(0);
	}

	begin = stm_now();
	std::sort(nums.begin(), nums.end());
	end = stm_now();

	sf::debugPrintLine("std: %.2fms", stm_ms(end - begin));

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
