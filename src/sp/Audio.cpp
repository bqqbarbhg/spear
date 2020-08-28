#include "Audio.h"

#include "sf/Float4.h"

namespace sp {

void AudioSampler::advanceMixStereo(float *dst, uint32_t numDst, AudioSource *source, const AudioMixOpts &opts)
{
	float samplesPerT = (float)opts.sampleRate * opts.volumeFadeDuration;

	uint32_t numDone = 0;
	while (numDone < numDst) {
		if (volumeT >= 0.9999f) {

			volumeT = 0.0f;
			if (started) {
				volumeSrc[0] = volumeDst[0];
				volumeSrc[1] = volumeDst[1];
			} else {
				volumeSrc[0] = opts.volumeNext[0];
				volumeSrc[1] = opts.volumeNext[1];
				started = true;
			}
			volumeDst[0] = opts.volumeNext[0];
			volumeDst[1] = opts.volumeNext[1];
		}

		float tLeft = 1.0f - volumeT;

		uint32_t numLeft = numDst - numDone;
		uint32_t samplesToDo = sf::min(numLeft, ((uint32_t)(samplesPerT * tLeft) + 1) & ~1u);
		volumeT += (float)samplesToDo / samplesPerT;

		advanceMixStereoImp(dst + numDone * 2, samplesToDo, source, opts);
		numDone += samplesToDo;
	}
}

void AudioSampler::advanceMixStereoImp(float *dstBuf, uint32_t numDst, AudioSource *source, const AudioMixOpts &opts)
{
	sf_assert(numDst % 2 == 0);

	uint32_t srcChannels = source->numChannels;
	uint32_t srcRate = source->sampleRate;
	double dstRate = (double)opts.sampleRate / opts.pitch;

	const constexpr uint32_t NumWorkData = 512;
	float workBuf[NumWorkData + AudioSource::AdvancePaddingInFloats];
	memcpy(workBuf, carryBuf, carryNumSamples * srcChannels * sizeof(float));

	uint32_t workFirstSample = carryFirstSample;
	uint32_t workNumSamples = carryNumSamples;

	float srcToDst = (float)(dstRate / (double)srcRate);
	float dstToSrc = (float)((double)srcRate / dstRate);

	sf::Float4 volA = sf::Float4(volumeSrc[0], volumeSrc[1], volumeSrc[0], volumeSrc[1]);
	sf::Float4 volB = sf::Float4(volumeDst[0], volumeDst[1], volumeDst[0], volumeDst[1]);
	sf::Float4 volSpan = volB - volA;

	sf::Float4 vol = volA + volSpan * volumeT;
	sf::Float4 dVolD2S = volSpan * (2.0f / opts.volumeFadeDuration / (float)opts.sampleRate);

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
				dst += src * vol;
				dst.storeu(dstPtr);

				vol += dVolD2S;
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
				sf::Float4::transpose22(src0, src1); 

				sf::Float4 delta(d0, d0, d1, d1);
				sf::Float4 src = src0 + (src1 - src0) * delta;
				sf::Float4 dst = sf::Float4::loadu(dstPtr);
				dst += src * vol;
				dst.storeu(dstPtr);

				vol += dVolD2S;
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

float *AudioLimiter::getStereoBuffer(uint32_t &requiredSamples, uint32_t numSamples, uint32_t sampleRate)
{
	uint32_t overflowLeft = numOverflowSamples - overflowOffset;
	if (overflowLeft >= numSamples) {
		requiredSamples = 0;
		return nullptr;
	}

	uint32_t windowSamples = ((uint32_t)(windowLengthSec * sampleRate) + 7) & ~7;
	uint32_t needWindows = (numSamples - overflowLeft + windowSamples - 1) / windowSamples;
	if (bufferEmpty) needWindows += 1;
	int32_t needSamples = (int32_t)(needWindows * windowSamples);
	if (needSamples < 0) {
		requiredSamples = 0;
		return nullptr;
	}

	requiredSamples = needSamples;
	return buffer.pushUninit(needSamples * 2);
}

void AudioLimiter::limitStereo(float *dstSamples, uint32_t numSamples, uint32_t sampleRate)
{
	float *dst = dstSamples;
	uint32_t samplesLeft = numSamples;

	uint32_t windowSamples = ((uint32_t)(windowLengthSec * sampleRate) + 7) & ~7;
	uint32_t overflowLeft = numOverflowSamples - overflowOffset;
	uint32_t needWindows = (numSamples - overflowLeft + windowSamples - 1) / windowSamples;
	uint32_t processWindows = needWindows;
	if (bufferEmpty) needWindows += 1;
	sf_assert(buffer.size == (numOverflowSamples + (processWindows + 1) * windowSamples) * 2);
	bufferEmpty = false;

	if (overflowLeft > 0) {
		uint32_t numToCopy = sf::min(overflowLeft, numSamples);
		memcpy(dst, buffer.data + overflowOffset * 2, numToCopy * 2 * sizeof(float));
		overflowOffset += numToCopy;
		samplesLeft -= numToCopy;
		dst += 2 * numToCopy;
	}

	if (samplesLeft == 0) return;

	float rcpWindowSamples = 1.0f / (float)windowSamples;

	float *alignedBuffer = buffer.data + numOverflowSamples * 2;
	overflowOffset = 0;
	numOverflowSamples = 0;

	float windowVolume, targetVolume;
	for (uint32_t i = 0; i < processWindows; i++) {
		uint32_t windowBase = i * windowSamples;

		// Find the maximum volume of the next window
		{
			sf::Float4 max0 = 0.0f, max1 = 0.0f;
			float *ptr = alignedBuffer + (windowBase + windowSamples) * 2;
			float *end = ptr + windowSamples * 2;
			sf_assert(end - buffer.data <= buffer.size);
			while (ptr != end) {
				sf::Float4 v0 = sf::Float4::loadu(ptr);
				sf::Float4 v1 = sf::Float4::loadu(ptr + 4);
				max0 = max0.max(v0.abs());
				max1 = max1.max(v1.abs());
				ptr += 8;
			}

			float maxSample = max0.max(max1).reduceMax();
			windowVolume = 1.0f / sf::max(maxSample, 1.0f);
		}

		// Linearly interpolate volume between
		targetVolume = sf::min(prevWindowVolume, windowVolume, sf::lerp(prevTargetVolume, 1.0f, volumeRecoverPerWindow));
		float delta = (targetVolume - prevTargetVolume) * rcpWindowSamples;
		sf::Float4 volume = sf::Float4(prevTargetVolume, prevTargetVolume, prevTargetVolume + delta, prevTargetVolume + delta);
		sf::ScalarAddFloat4 deltaVolume = delta * 2.0f;

		if (samplesLeft >= windowSamples) {
			sf::Float4 maxS = 0.0f;
			float *ptr = alignedBuffer + windowBase * 2;
			float *end = ptr + windowSamples * 2;
			sf_assert(end - buffer.data <= buffer.size);
			while (ptr != end) {
				sf::Float4 v = sf::Float4::loadu(ptr);
				v *= volume;
				v.storeu(dst);
				volume += deltaVolume;
				ptr += 4;
				dst += 4;
			}
			samplesLeft -= windowSamples;
		} else if (samplesLeft > 0) {
			numOverflowSamples = windowSamples - samplesLeft;

			sf::Float4 maxS = 0.0f;
			float *base = alignedBuffer + windowBase * 2;
			float *ptr = base;
			float *end = ptr + windowSamples * 2;
			sf_assert(end - buffer.data <= buffer.size);
			while (ptr != end) {
				sf::Float4 v = sf::Float4::loadu(ptr);
				v *= volume;
				v.storeu(ptr);
				volume += deltaVolume;
				ptr += 4;
			}

			memcpy(dst, base, samplesLeft * 2 * sizeof(float));
			dst += samplesLeft * 2;
			samplesLeft = 0;
		}

		prevTargetVolume = targetVolume;
		prevWindowVolume = windowVolume;
	}

	uint32_t copyUnits = (windowSamples + numOverflowSamples) * 2;
	memmove(buffer.data, buffer.data + buffer.size - copyUnits, copyUnits * sizeof(float));
	buffer.size = copyUnits;

	sf_assert(dst == dstSamples + numSamples * 2);
}

}

