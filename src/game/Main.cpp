#include "sp/GameMain.h"

#include "sf/Sort.h"
#include "sf/String.h"

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

static void doArray(sf::Array<int> &nums, int pattern, int size)
{
	nums.clear();
	uint32_t rng = 1;
	for (int i = 0; i < size; i++) {
		rng ^= rng << 13;
		rng ^= rng >> 17;
		rng ^= rng << 5;

		switch (pattern) {
		case 0: nums.push(i); break;
		case 1: nums.push(-i); break;
		case 2: nums.push(i < size / 2 ? i : size - i); break;
		case 3: nums.push(i > size / 2 ? i : size - i); break;
		case 4: nums.push(rng % 2); break;
		case 5: nums.push(rng % 10); break;
		case 6: nums.push(rng); break;
		case 7: nums.push(0); break;
		case 8: nums.push((i + 1) % size); break;
		case 9: nums.push(i + 1 < size ? i : 0); break;
		case 10: nums.push(i % 2 != 0 ? i : size - i); break;
		}
	}
}

struct DataOrder
{
	uint64_t data[1];
	int order;
};

static void doArray(sf::Array<DataOrder> &data, sf::Slice<int> ints)
{
	data.clear();
	for (int i : ints) {
		data.push().order = i;
	}
}

static void doArray(sf::Array<sf::StringBuf> &strs, sf::Slice<int> ints)
{
	strs.clear();
	for (int i : ints) {
		strs.push().format("%d", i);
	}
}

void spInit()
{
	sf::Array<int> nums;
	sf::Array<sf::StringBuf> strs;
	sf::Array<DataOrder> data;

	const char *patterns[] = {
		"sequential", "reverse", "pipe", "revpipe", "random%2", "random%10", "random", "zeros", "rotated",
		"lastzero", "interleaved",
	};
	const int sizes[] = {
		1000, 10000, 100000, 
#ifndef _DEBUG
		// 1000000,
#endif
	};

	double iavg = 0.0, savg = 0.0, davg = 0.0;
	uint32_t num = 0;

	for (int size : sizes) {
		for (int pat = 0; pat < sf_arraysize(patterns); pat++) {
			sf::StringBuf label;
			label.format("int %s %d", patterns[pat], size);

			uint64_t begin;
			uint64_t sftime, stdtime;
			doArray(nums, pat, size);
			doArray(strs, nums);

			begin = stm_now();
			sf::sort(nums);
			sftime = stm_now() - begin;

			for (size_t i = 1; i < nums.size; i++) {
				sf_assert(nums[i - 1] <= nums[i]);
			}

			doArray(nums, pat, size);

			begin = stm_now();
			std::sort(nums.begin(), nums.end());
			stdtime = stm_now() - begin;
			sf::debugPrintLine("%-24s sf %8.2fms    std %8.2fms    %5.1f%%", label.data,
				stm_ms(sftime), stm_ms(stdtime), stm_ms(sftime) / stm_ms(stdtime) * 100.0);

			iavg += stm_ms(sftime) / stm_ms(stdtime);
			num++;
		}
	}

	for (int size : sizes) {
		for (int pat = 0; pat < sf_arraysize(patterns); pat++) {
			sf::StringBuf label;
			label.format("str %s %d", patterns[pat], size);

			uint64_t begin;
			uint64_t sftime, stdtime;
			doArray(nums, pat, size);
			doArray(strs, nums);

			begin = stm_now();
			sf::sort(strs);
			sftime = stm_now() - begin;

			for (size_t i = 1; i < nums.size; i++) {
				sf_assert(!(strs[i] < strs[i - 1]));
			}

			doArray(nums, pat, size);
			doArray(strs, nums);

			begin = stm_now();
			std::sort(strs.begin(), strs.end());
			stdtime = stm_now() - begin;

			sf::debugPrintLine("%-24s sf %8.2fms    std %8.2fms    %5.1f%%", label.data,
				stm_ms(sftime), stm_ms(stdtime), stm_ms(sftime) / stm_ms(stdtime) * 100.0);

			savg += stm_ms(sftime) / stm_ms(stdtime);
		}
	}

	for (int size : sizes) {
		for (int pat = 0; pat < sf_arraysize(patterns); pat++) {
			sf::StringBuf label;
			label.format("dat %s %d", patterns[pat], size);

			uint64_t begin;
			uint64_t sftime, stdtime;
			doArray(nums, pat, size);
			doArray(data, nums);

			begin = stm_now();
			sf::sortBy(data, [](const DataOrder &o) { return o.order; });
			sftime = stm_now() - begin;

			for (size_t i = 1; i < nums.size; i++) {
				sf_assert(data[i - 1].order <= data[i].order);
			}

			doArray(nums, pat, size);
			doArray(data, nums);

			begin = stm_now();
			std::sort(data.begin(), data.end(), [](const DataOrder &l, const DataOrder &r) {
				return l.order < r.order;
			});
			stdtime = stm_now() - begin;

			sf::debugPrintLine("%-24s sf %8.2fms    std %8.2fms    %5.1f%%", label.data,
				stm_ms(sftime), stm_ms(stdtime), stm_ms(sftime) / stm_ms(stdtime) * 100.0);

			davg += stm_ms(sftime) / stm_ms(stdtime);
		}
	}

	sf::debugPrintLine("int avg: %5.1f%%", iavg / num * 100.0);
	sf::debugPrintLine("str avg: %5.1f%%", savg / num * 100.0);
	sf::debugPrintLine("dat avg: %5.1f%%", davg / num * 100.0);

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
