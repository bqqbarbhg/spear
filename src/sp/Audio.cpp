#include "Audio.h"

#include "sf/Float4.h"

#include "sf/ext/mx/mx_platform.h"

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

		advanceMixStereoImp(dst + numDone * 2, samplesToDo, source, opts);
		volumeT += (float)samplesToDo / samplesPerT;
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

	float volumeStep = 1.0f / opts.volumeFadeDuration / (float)opts.sampleRate;
	sf::Float4 vol = volA + volSpan * sf::Float4(volumeT, volumeT, volumeT + volumeStep, volumeT + volumeStep);
	sf::Float4 dVolD2S = volSpan * (2.0f * volumeStep);

	float *dstPtr = dstBuf, *dstEnd = dstPtr + numDst * 2;
	while (dstPtr != dstEnd) {
		sf_assert(dstPtr < dstEnd);
		uint32_t dstLeft = (uint32_t)(dstEnd - dstPtr) >> 1;
		uint32_t minSample = (uint32_t)srcSampleTime;
		uint32_t maxSample = (uint32_t)(srcSampleTime + (double)(dstLeft + 4) * dstToSrc) + 4;

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
			uint32_t numSafeSamples = sf::min(dstLeft, (uint32_t)((float)(workNumSamples - 2) * srcToDst - t - 2.0f)) >> 1;
			srcSampleTime += dstToSrc * (float)(numSafeSamples * 2);

			while (numSafeSamples-- > 0) {
				uint32_t i0, i1;
				float d0, d1;
				i0 = (uint32_t)t; d0 = t - (float)i0; t += dstToSrc;
				i1 = (uint32_t)t; d1 = t - (float)i1; t += dstToSrc;

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
			uint32_t numSafeSamples = sf::min(dstLeft, (uint32_t)((float)(workNumSamples - 2) * srcToDst - t - 2.0f)) >> 1;
			srcSampleTime += dstToSrc * (float)(numSafeSamples * 2);

			while (numSafeSamples-- > 0) {
				uint32_t i0, i1;
				float d0, d1;
				i0 = (uint32_t)t; d0 = t - (float)i0; t += dstToSrc;
				i1 = (uint32_t)t; d1 = t - (float)i1; t += dstToSrc;

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

void AudioLimiter::reset()
{
	buffer.clear();
	prevWindowVolume = 1.0f;
	prevTargetVolume = 1.0f;
	bufferEmpty = true;
	overflowOffset = 0;
	numOverflowSamples = 0;
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
			sf_assert(end - buffer.data <= (ptrdiff_t)buffer.size);
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
			sf_assert(end - buffer.data <= (ptrdiff_t)buffer.size);
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
			sf_assert(end - buffer.data <= (ptrdiff_t)buffer.size);
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

void BeginLoopAudioSource::seek(uint32_t sample)
{
	// Only support seeking into the beginning for now
	begin->seek(sample);
	loop->seek(0);
	inLoop = false;
}

BeginLoopAudioSource::BeginLoopAudioSource()
{
}

BeginLoopAudioSource::BeginLoopAudioSource(sf::Box<AudioSource> begin_, sf::Box<AudioSource> loop_)
	: begin(std::move(begin_))
	, loop(std::move(loop_))
{
	numChannels = begin->numChannels;
	sampleRate = begin->sampleRate;
	sf_assert(numChannels == loop->numChannels);
	sf_assert(sampleRate == loop->sampleRate);
}

uint32_t BeginLoopAudioSource::advance(uint32_t sample, float *dst, uint32_t num)
{
	uint32_t numTotal = 0;

	if (!inLoop) {
		uint32_t numBegin = begin->advance(sample, dst, num);
		numTotal += numBegin;
		sample += numBegin;
		dst += numBegin * numChannels;
		num -= numBegin;
		if (numBegin < num) {
			inLoop = true;
			loopBeginSample = sample;
			sf::debugPrintLine("BEGIN");
		}
	}

	while (inLoop && num > 0) {
		uint32_t localSample = sample - loopBeginSample;
		uint32_t numLoop = loop->advance(localSample, dst, num);
		numTotal += numLoop;
		sample += numLoop;
		dst += numLoop * numChannels;
		num -= numLoop;
		if (num > 0) {
			loop->seek(0);
			loopBeginSample = sample;
			sf::debugPrintLine("LOOP");
		}
	}

	return numTotal;
}

void BeginLoopEndAudioSource::stop()
{
	mxa_cas32(&impStopFlag, 0, 1);
}

bool BeginLoopEndAudioSource::unstop()
{
	if (mxa_load32_nf(&impStopFlag) == 0) return true;
	return mxa_cas32(&impStopFlag, 1, 0);
}

void BeginLoopEndAudioSource::seek(uint32_t sample)
{
	// TODO: This would be doable by tracking transitions,
	// but I don't feel like it
	if (sample == 0) {
		begin->seek(sample);
		loop->seek(0);
		if (end) end->seek(0);
		impState = Begin;
		impBeginSample = 0;
	}
}

uint32_t BeginLoopEndAudioSource::advance(uint32_t sample, float *dst, uint32_t num)
{
	uint32_t numTotal = 0;

	if (impState == Begin) {
		uint32_t numBegin = begin->advance(sample, dst, num);
		numTotal += numBegin;
		if (numBegin == num) return numTotal;

		impBeginSample = sample + numBegin;
		sample += numBegin;
		dst += numBegin * 2;
		num -= numBegin;
		if (mxa_cas32(&impStopFlag, 1, 2)) {
			impState = End;
			sample += numBegin;
		} else {
			impState = Loop;
		}
	}

	while (impState == Loop) {
		uint32_t localSample = sample - impBeginSample;
		uint32_t numLoop = loop->advance(localSample, dst, num);
		numTotal += numLoop;
		if (numLoop == num) return numTotal;

		impBeginSample = sample + numLoop;
		dst += numLoop * 2;
		num -= numLoop;
		if (mxa_cas32(&impStopFlag, 1, 2)) {
			impState = End;
			sample += numLoop;
		} else {
			loop->seek(0);
		}
	}

	if (impState == End) {
		uint32_t localSample = sample - impBeginSample;
		uint32_t numEnd = end->advance(localSample, dst, num);
		numTotal += numEnd;
	}

	return numTotal;
}

void InterruptLoopAudioSource::stop()
{
	mxa_cas32(&impStopFlag, 0, 1);
}

bool InterruptLoopAudioSource::unstop()
{
	if (mxa_load32_nf(&impStopFlag) == 0) return true;
	return mxa_cas32(&impStopFlag, 1, 0);
}

void InterruptLoopAudioSource::seek(uint32_t sample)
{
	// TODO: This would be doable by tracking transitions,
	// but I don't feel like it
	if (sample == 0) {
		loop->seek(0);
		end->seek(0);
		impStopFlag = 0;
		hasFadeBuffer = false;
		atEnd = false;
		totalSamples = 0;
		fadeBufferFirstSample = 0;
	} else {
		sf_failf("TODO");
	}
}

uint32_t InterruptLoopAudioSource::advance(uint32_t sample, float *dst, uint32_t num)
{
	uint32_t beginSample = sample;

	if (!hasFadeBuffer) {
		uint32_t stopFlag = mxa_load32_nf(&impStopFlag);
		bool shouldStop = false;
		if (stopFlag == 2) {
			shouldStop = true;
		} else if (stopFlag == 1) {
			shouldStop = mxa_cas32(&impStopFlag, 1, 2);
		}

		uint32_t numToLoop = num;
		bool makeCrossfade = false;
		if (shouldStop) {
			uint64_t samplesUntilNextInterrupt = (uint64_t)interruptInterval - totalSamples % (uint64_t)interruptInterval;
			if (samplesUntilNextInterrupt <= num) {
				numToLoop = (uint32_t)samplesUntilNextInterrupt;
				makeCrossfade = true;
			}
		}

		uint32_t numLoop = loop->advance(sample, dst, numToLoop);
		sample += numToLoop;
		dst += numToLoop * numChannels;
		num -= numToLoop;

		if (makeCrossfade) {
			// TODO: In-place with smaller buffer
			float localFadeBuffer[1024];
			sf_assert(fadeBuffer.size <= 1024);
			uint32_t fadeSamples = fadeBuffer.size / numChannels;

			loop->advance(sample, localFadeBuffer, fadeSamples);
			end->advance(0, fadeBuffer.data, fadeSamples);

			// TODO: Float4
			float dt = 1.0f / (float)fadeSamples;
			if (numChannels == 1) {
				float t = 0.0f;
				float *a = localFadeBuffer;
				float *b = fadeBuffer.data;
				for (uint32_t i = 0; i < fadeSamples; i++) {
					b[0] = a[0] * (1.0f - t) + b[0] * t;
					a++;
					b++;
					t += dt;
				}
			} else if (numChannels == 2) {
				float t = 0.0f;
				float *a = localFadeBuffer;
				float *b = fadeBuffer.data;
				for (uint32_t i = 0; i < fadeSamples; i++) {
					b[0] = a[0] * (1.0f - t) + b[0] * t;
					b[1] = a[1] * (1.0f - t) + b[1] * t;
					a += 2;
					b += 2;
					t += dt;
				}
			}

			fadeBufferFirstSample = sample;
			hasFadeBuffer = true;
		}
	}

	if (!atEnd && hasFadeBuffer) {
		uint32_t offset = sample - fadeBufferFirstSample;
		uint32_t fadeLeft = fadeBuffer.size / numChannels - offset;
		uint32_t numToCopy = sf::min(fadeLeft, num);
		memcpy(dst, fadeBuffer.data + offset * numChannels, sizeof(float) * numChannels * numToCopy);
		sample += numToCopy;
		dst += numToCopy * numChannels;
		num -= numToCopy;
		if (num > 0) {
			atEnd = true;
		}
	}

	if (atEnd && num > 0) {
		uint32_t localSample = sample - fadeBufferFirstSample;
		uint32_t numEnd = end->advance(sample, dst, num);
		sample += numEnd;
		dst += numEnd * numChannels;
		num -= numEnd;
	}

	uint32_t numRead = sample - beginSample;
	totalSamples += numRead;
	return numRead;
}

void SwappingAudioSource::play(uint32_t sourceIndex)
{
	mxa_cas32(&impShouldStart[sourceIndex], 0, 1);
}

void SwappingAudioSource::unplay(uint32_t sourceIndex)
{
	mxa_cas32(&impShouldStart[sourceIndex], 1, 0);
}

bool SwappingAudioSource::isPlaying(uint32_t sourceIndex) const
{
	return mxa_load32(&impPlaying[sourceIndex]) != 0;
}

void SwappingAudioSource::seek(uint32_t sample)
{
	sf_failf("TODO");
}

uint32_t SwappingAudioSource::advance(uint32_t sample, float *dst, uint32_t num)
{
	uint32_t beginSample = sample;
	uint32_t originalNum = num;

	while (num > 0) {
		uint32_t numToRead = num;

		bool crossesBoundary = false;
		uint64_t samplesUntilNextInterrupt = (uint64_t)swapInterval - totalSamples % (uint64_t)swapInterval;
		if (samplesUntilNextInterrupt <= num) {
			numToRead = (uint32_t)samplesUntilNextInterrupt;
			crossesBoundary = true;
		}

		float buf[512];
		uint32_t chunkSize = sf_arraysize(buf) / numChannels;
		uint32_t base = 0;
		for (uint32_t base = 0; base < numToRead; base += chunkSize) {
			uint32_t chunkActual = sf::min(num, chunkSize);

			uint32_t numPlaying = 0;
			for (uint32_t srcI = 0; srcI < 2; srcI++) {
				if (!mxa_load32_nf(&impPlaying[srcI])) continue;
				uint32_t localSample = (uint32_t)(totalSamples - startSample[srcI]);

				uint32_t localNum;
				if (numPlaying == 0) {
					localNum = source[srcI]->advance(localSample, dst, chunkActual);
					memset(dst + localNum*numChannels, 0, (chunkActual - localNum) * numChannels * sizeof(float));
				} else {
					localNum = source[srcI]->advance(localSample, buf, chunkActual);
					uint32_t floatsToAdd = localNum * numChannels;
					for (uint32_t i = 0; i < floatsToAdd; i++) {
						dst[i] += buf[i];
					}
				}
				numPlaying++;

				if (localNum < chunkActual) {
					mxa_cas32(&impPlaying[srcI], 1, 0);
				}
			}

			if (numPlaying == 0) {
				memset(dst, 0, chunkActual * numChannels * sizeof(float));
			}

			sample += chunkActual;
			dst += chunkActual * numChannels;
			num -= chunkActual;
			totalSamples += chunkActual;
		}

		if (crossesBoundary) {
			for (uint32_t srcI = 0; srcI < 2; srcI++) {
				if (mxa_load32_nf(&impPlaying[srcI])) continue;
				if (mxa_load32_nf(&impShouldStart[srcI]) == 0) continue;
				if (!mxa_cas32(&impShouldStart[srcI], 1, 0)) continue;

				startSample[srcI] = totalSamples;
				source[srcI]->seek(0);
				mxa_cas32(&impPlaying[srcI], 0, 1);
			}
		}
	}

	(void)beginSample;
	sf_assert(sample - beginSample == originalNum);
	return originalNum;
}

}

