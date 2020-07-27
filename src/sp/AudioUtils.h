#pragma once

#include "sf/Float4.h"

namespace sp {

void getAudioFloatDelta(sf::Float4 &init, sf::ScalarFloat4 &inc, float current, float next, uint32_t numSamples)
{
	float sampleDelta = (next - current) / numSamples;
	init = sf::Float4(current) + sf::Float4(0.0f, 1.0f, 2.0f, 3.0f) * sampleDelta;
	inc = sampleDelta * 4.0f;
}

}
