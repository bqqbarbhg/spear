#include "Audio.h"

#include "sf/Float4.h"

namespace sp {

void AudioSampler::advanceMixStereo(float *dstBuf, uint32_t numDst, AudioSource *source, const AudioMixOpts &opts)
{
	sf_assert(numDst % 2 == 0);

	uint32_t srcChannels = source->numChannels;
	uint32_t srcRate = source->sampleRate;
	double dstRate = (double)opts.sampleRate / opts.pitch;

	const constexpr uint32_t NumWorkData = 4096*2;
	float workBuf[NumWorkData + 16];
	memcpy(workBuf, carryBuf, carryNumSamples * srcChannels * sizeof(float));

	uint32_t workFirstSample = carryFirstSample;
	uint32_t workNumSamples = carryNumSamples;

	float srcToDst = (float)(dstRate / (double)srcRate);
	float dstToSrc = (float)((double)srcRate / dstRate);

	sf::Float4 volA = sf::Float4(prevVolume[0], prevVolume[1], prevVolume[0], prevVolume[1]);
	sf::Float4 volB = sf::Float4(opts.volume[0], opts.volume[1], opts.volume[0], opts.volume[1]);
	sf::Float4 volMin = volA.min(volB);
	sf::Float4 volMax = volA.max(volB);

	float volL = prevVolume[0];
	float volR = prevVolume[1];
	float dVolLdS = (opts.volume[0] - volL) * (opts.volumeFadeSpeed/(float)dstRate);
	float dVolRdS = (opts.volume[1] - volR) * (opts.volumeFadeSpeed/(float)dstRate);
	prevVolume[0] = sf::clamp(volL + dVolLdS * (float)numDst, volMin.getX(), volMax.getX());
	prevVolume[1] = sf::clamp(volR + dVolRdS * (float)numDst, volMin.getY(), volMax.getY());

	sf::Float4 vol = sf::Float4(volL, volR, volL + dVolLdS, volR + dVolRdS);
	sf::Float4 dVolD2S = sf::Float4(dVolLdS, dVolRdS, dVolLdS, dVolRdS) * 2.0f;

	float *dstPtr = dstBuf, *dstEnd = dstPtr + numDst * 2;
	while (dstPtr != dstEnd) {
		sf_assert(dstPtr < dstEnd);
		uint32_t dstLeft = (uint32_t)(dstEnd - dstPtr) >> 1;
		uint32_t minSample = (uint32_t)srcSampleTime;
		uint32_t maxSample = (uint32_t)(srcSampleTime + (double)(dstLeft + 4.0f) * dstToSrc);

		uint32_t workLastSample = workFirstSample + workNumSamples;
		int32_t missingSamples = (int32_t)maxSample - (int32_t)workLastSample;

		missingSamples = sf::min(missingSamples, (int32_t)(NumWorkData / srcChannels - workNumSamples));
		while (missingSamples > 0) {
			uint32_t numRead = source->advance(srcAdvanceSample, workBuf + workNumSamples * srcChannels, missingSamples);
			srcAdvanceSample += numRead;
			workNumSamples += numRead;
			missingSamples -= (int32_t)numRead;
			if (missingSamples > 0) {
				if (opts.loop) {
					srcAdvanceSample = 0;
					srcSampleTime -= workFirstSample;
					workFirstSample = 0;
					source->seek(0);
					loopCount++;
				} else {
					memset(workBuf + workNumSamples * srcChannels, 0, missingSamples * srcChannels * sizeof(float));
					workNumSamples += (uint32_t)missingSamples;
					missingSamples = 0;
					ended = true;
				}
			}
		}

		if (srcChannels == 1) {
			float t = (float)(srcSampleTime - (double)workFirstSample);
			uint32_t numSafeSamples = sf::min(dstLeft, (uint32_t)((float)workNumSamples * srcToDst - t - 2.0f)) >> 1;
			srcSampleTime += dstToSrc * (float)(numSafeSamples * 2);

			while (numSafeSamples-- > 0) {
				uint32_t i0, i1;
				float d0, d1;
				i0 = (uint32_t)t; d0 = (float)i0 - t; t += dstToSrc;
				i1 = (uint32_t)t; d1 = (float)i1 - t; t += dstToSrc;

				float a0 = workBuf[i0], b0 = workBuf[i0 + 1];
				float a1 = workBuf[i1], b1 = workBuf[i1 + 1];

				sf::Float4 src0(a0, a0, a1, a1);
				sf::Float4 src1(b0, b0, b1, b1);

				sf::Float4 delta(d0, d0, d1, d1);
				sf::Float4 src = src0 + (src1 - src0) * delta;
				sf::Float4 dst = sf::Float4::loadu(dstPtr);
				// dst += src * vol;
				dst += src;
				dst.storeu(dstPtr);

				vol += dVolD2S;
				vol = vol.min(volMax).max(volMin);
				dst.storeu(dstPtr);
				dstPtr += 4;
			}
		} else {
			float t = (float)(srcSampleTime - (double)workFirstSample);
			uint32_t numSafeSamples = sf::min(dstLeft, (uint32_t)((float)workNumSamples * srcToDst - t - 2.0f)) >> 1;
			srcSampleTime += dstToSrc * (float)(numSafeSamples * 2);

			while (numSafeSamples-- > 0) {
				uint32_t i0, i1;
				float d0, d1;
				i0 = (uint32_t)t; d0 = (float)i0 - t; t += dstToSrc;
				i1 = (uint32_t)t; d1 = (float)i1 - t; t += dstToSrc;

				sf::Float4 src0 = sf::Float4::loadu(workBuf + i0*2); // LA0 RA0 LA1 RA1 <-T- (LA0 RA0 LB0 RB0)
				sf::Float4 src1 = sf::Float4::loadu(workBuf + i1*2); // LB0 RB0 LB1 RB1 <-T- (LA1 RA1 LB1 RB1)
				sf::Float4::transpose2(src0, src1); 

				sf::Float4 delta(d0, d0, d1, d1);
				sf::Float4 src = src0 + (src1 - src0) * delta;
				sf::Float4 dst = sf::Float4::loadu(dstPtr);
				dst += src * vol;
				dst.storeu(dstPtr);

				vol += dVolD2S;
				vol = vol.min(volMax).max(volMin);
				dstPtr += 4;
			}
		}

		uint32_t firstNeededWorkSample = (uint32_t)srcSampleTime - workFirstSample;
		uint32_t moveNumSamples = workNumSamples - firstNeededWorkSample;
		sf_assert(moveNumSamples * srcChannels <= sf_arraysize(carryBuf));
		memmove(workBuf, workBuf + firstNeededWorkSample*srcChannels, moveNumSamples*srcChannels*sizeof(float));
		workFirstSample += firstNeededWorkSample;
		workNumSamples = moveNumSamples;
	}

	carryFirstSample = workFirstSample;
	carryNumSamples = workNumSamples;
	memcpy(carryBuf, workBuf, workNumSamples*srcChannels*sizeof(float));
}

}

