#pragma once

#include "Asset.h"

namespace sp {

struct CompressedSound : Asset
{
	static AssetType SelfType;
	using PropType = NoAssetProps;

	uint32_t sampleRate = 0;
	uint32_t numSamples = 0;
	uint32_t numChannels = 0;
	double lengthSeconds = 0.0;

	uint32_t decodeStereo(float *left, float *right, uint32_t numSamples, bool loop);
};

using CompressedSoundRef = Ref<CompressedSound>;

}
