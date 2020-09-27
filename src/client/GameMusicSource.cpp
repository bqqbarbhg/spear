#include "GameMusicSource.h"

#include "sf/ext/mx/mx_platform.h"

namespace cl {

GameMusicSource::GameMusicSource(sf::Slice<sf::Box<sp::AudioSource>> sources)
{
	numChannels = 2;
	sampleRate = 44100;

	sf_assert(sources.size == 2);
	for (uint32_t i = 0; i < 2; i++) {
		Track &track = tracks[i];
		track.source = sources[i];

		track.fadeIn.resize(fadeSamples * 2);
		track.source->advance(0, track.fadeIn.data, fadeSamples);
		track.sample = fadeSamples;
	}
}

void GameMusicSource::updateStateImp()
{
	sf_assert(sampleOffset % swapInteval == swapInteval - fadeSamples);

	uint32_t next = mxa_load32_nf(&atomicNextTrack);
	if (next != nextTrack && fadeBeginOffset == UINT64_MAX) {
		prevTrack = nextTrack;
		nextTrack = next;

		Track &track = tracks[next];
		track.sample = fadeSamples;
		track.source->seek(fadeSamples);
		fadeBeginOffset = sampleOffset;
	}
}

void GameMusicSource::requestTrack(uint32_t index)
{
	uint32_t prev;
	do {
		prev = mxa_load32_nf(&atomicNextTrack);
		if (prev == index) break;
	} while (mxa_cas32_nf(&atomicNextTrack, prev, index));
}

void GameMusicSource::seek(uint32_t sample)
{
	sf_failf("TODO");
}

uint32_t GameMusicSource::advance(uint32_t sample, float *dst, uint32_t num)
{
	float *startDst = dst;
	uint32_t numLeft = num;

	sf_assert(sampleRate == 44100);
	sf_assert(numChannels == 2);

	while (numLeft > 0) {

		if (fadeBeginOffset != UINT64_MAX) {
			sf_assert(sampleOffset >= fadeBeginOffset);
			uint32_t fadeTime = (uint32_t)(sampleOffset - fadeBeginOffset);
			sf_assert(fadeTime < fadeSamples);
			uint32_t fadeToProcess = sf::min(fadeSamples - fadeTime, numLeft);
			sf_assert(fadeToProcess > 0);

			Track &prev = tracks[prevTrack];
			Track &next = tracks[nextTrack];

			float dt = 1.0f / 1000.0f;
			float t = sf::min((float)fadeTime * dt, 1.0f);

			if (t < 1.0f) {
				prev.source->advance(prev.sample, dst, fadeToProcess);
				prev.sample += fadeToProcess;
			}

			const float *src = next.fadeIn.data + fadeTime * 2;
			for (uint32_t i = 0; i < fadeToProcess; i++) {
				float it = 1.0f - t;
				dst[0] = dst[0] * it + src[0] * t;
				dst[1] = dst[1] * it + src[1] * t;
				dst += 2;
				src += 2;
				t = sf::min(t + dt, 1.0f);
			}

			numLeft -= fadeToProcess;
			sampleOffset += fadeToProcess;
			if (sampleOffset == fadeBeginOffset + fadeSamples) {
				fadeBeginOffset = UINT64_MAX;
			}
		} else {
			uint32_t cycleLeft = swapInteval - (uint32_t)(sampleOffset % (uint64_t)swapInteval);

			uint32_t loopToProcess;
			if (cycleLeft > fadeSamples) {
				loopToProcess = cycleLeft - fadeSamples;
			} else {
				loopToProcess = cycleLeft;
			}
			loopToProcess = sf::min(loopToProcess, numLeft);

			Track &next = tracks[nextTrack];

			if (loopToProcess > 0) {
				next.source->advance(next.sample, dst, loopToProcess);
				next.sample += loopToProcess;
			}

			dst += loopToProcess * numChannels;
			numLeft -= loopToProcess;
			sampleOffset += loopToProcess;

			cycleLeft -= loopToProcess;
			if (cycleLeft == fadeSamples) {
				updateStateImp();
			}
		}

	}

	sf_assert(dst == startDst + num * 2);
	return num;
}

}
