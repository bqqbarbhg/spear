#pragma once

#include "sf/Array.h"
#include "sp/Audio.h"

namespace cl {

struct GameMusicSource : sp::AudioSource
{
	static const uint32_t fadeSamples = 5000;

	struct Track
	{
		uint32_t sample = 0;
		sf::Array<float> fadeIn;
		sf::Box<sp::AudioSource> source;
	};

	Track tracks[2];

	uint64_t fadeBeginOffset = UINT64_MAX;
	uint64_t sampleOffset = 0;

	uint32_t prevTrack = 0;
	uint32_t nextTrack = 0;

	uint32_t swapInteval = 2 * 100800;

	uint32_t atomicNextTrack = 0;

	GameMusicSource(sf::Slice<sf::Box<sp::AudioSource>> sources);

	void updateStateImp();

	void requestTrack(uint32_t index);

	virtual void seek(uint32_t sample) override;
	virtual uint32_t advance(uint32_t sample, float *dst, uint32_t num) override;
};

}
