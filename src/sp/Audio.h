#pragma once

#include "sf/Base.h"

namespace sp {

struct AudioSource
{
	static const constexpr uint32_t AdvancePaddingInFloats = 8;

	uint32_t sampleRate;
	uint32_t numChannels;

	virtual void seek(uint32_t sample) = 0;

	// Allowed to read past `dst` by AdvancePaddingInFloats floats!
	virtual uint32_t advance(uint32_t sample, float *dst, uint32_t num) = 0;
};

struct AudioMixOpts
{
	bool loop = false;
	uint32_t sampleRate = 44100;
	float volume[2] = { 1.0f, 1.0f };
	float volumeFadeSpeed = 100.0f;
	float pitch = 1.0f;
};

struct AudioSampler
{
	float carryBuf[16];
	uint32_t carryFirstSample = 0;
	uint32_t carryNumSamples = 0;

	uint32_t srcAdvanceSample = 0;
	double srcSampleTime = 0.0;

	uint32_t loopCount = 0;
	bool ended = false;

	float prevVolume[2];

	void advanceMixStereo(float *dst, uint32_t numDst, AudioSource *source, const AudioMixOpts &opts);
};

}
