#include "Sound.h"

#include "ContentFile.h"
#include "sf/HashMap.h"
#include "sf/Array.h"

#include "sf/Float4.h"

#include "ext/stb/stb_vorbis.h"

namespace sp {

struct SoundImp : Sound
{
	virtual void assetStartLoading() final;
	virtual void assetUnload() final;

	sf::Box<void> data;
	size_t size = 0;

	sf::Array<sf::Box<AudioSource>> sharedSources;
};

struct VorbisSource : AudioSource
{
	sf::Box<void> dataRef;
	stb_vorbis *vorbis = nullptr;
	sf::Array<char> tempMemory;

	VorbisSource(const sf::Box<void> &data, uint32_t offset, size_t size, uint32_t tempMemorySize, uint32_t sampleRate, uint32_t numChannels)
		: dataRef(data)
	{
		this->sampleRate = sampleRate;
		this->numChannels = numChannels;

		tempMemory.resizeUninit(tempMemorySize);

		stb_vorbis_alloc alloc;
		alloc.alloc_buffer = tempMemory.data;
		alloc.alloc_buffer_length_in_bytes = (int)tempMemory.size;

		int error = 0;
		vorbis = stb_vorbis_open_memory((const unsigned char*)data.ptr + offset, (int)size, &error, &alloc);
	}

	virtual void seek(uint32_t sample) override
	{
		stb_vorbis_seek(vorbis, sample);
	}

	virtual uint32_t advance(uint32_t sample, float *dst, uint32_t num) override
	{
		return (uint32_t)stb_vorbis_get_samples_float_interleaved(vorbis, (int)numChannels, dst, num*numChannels);
	}
};

struct Pcm16Source : AudioSource
{
	sf::Box<void> dataRef;
	uint32_t numSamples;
	void *data;

	Pcm16Source(const sf::Box<void> &data, uint32_t offset, uint32_t sampleRate, uint32_t numChannels, uint32_t numSamples)
		: dataRef(data), numSamples(numSamples)
	{
		this->sampleRate = sampleRate;
		this->numChannels = numChannels;
		this->data = (char*)data.ptr + offset;
	}

	virtual void seek(uint32_t sample) override
	{
	}

	virtual uint32_t advance(uint32_t sample, float *dst, uint32_t num) override
	{
		uint32_t numToRead = (uint32_t)sf::clamp((int32_t)numSamples - (int32_t)sample, 0, (int32_t)num);

		const int16_t *ptr = (const int16_t*)data + (sample * numChannels);
		uint32_t numUnits = numToRead * numChannels;
#if SF_FLOAT4_SCALAR
		float scale = 1.0f/32768.0f;
		for (uint32_t i = 0; i < numUnits; i += 8) {
			dst[0] = (float)ptr[0] * scale;
			dst[1] = (float)ptr[1] * scale;
			dst[2] = (float)ptr[2] * scale;
			dst[3] = (float)ptr[3] * scale;
			dst[4] = (float)ptr[4] * scale;
			dst[5] = (float)ptr[5] * scale;
			dst[6] = (float)ptr[6] * scale;
			dst[7] = (float)ptr[7] * scale;
			dst += 8;
			ptr += 8;
		}
#else
		sf::ScalarFloat4 scale = 1.0f/32768.0f;
		for (uint32_t i = 0; i < numUnits; i += 8) {
			sf::Float4 a, b;
			sf::Float4::load8xi16(a, b, ptr);

			a *= scale;
			b *= scale;
			a.storeu(dst + 0);
			b.storeu(dst + 4);

			ptr += 8;
			dst += 8;
		}
#endif

		return numToRead;
	}
};

AssetType Sound::SelfType = { "Sound", sizeof(SoundImp), sizeof(Sound::PropType),
	[](Asset *a) { new ((SoundImp*)a) SoundImp(); }
};

static void loadImp(void *user, const ContentFile &file)
{
	SoundImp *imp = (SoundImp*)user;

	if (file.size == 0) {
		imp->assetFailLoading();
		return;
	}

	spsound_util su;
	spsound_util_init(&su, file.data, file.size);
	spsound_header header = spsound_decode_header(&su);
	if (!spfile_util_failed(&su.file)) {
		imp->takes.resizeUninit(header.info.num_takes);
		spsound_decode_takes_to(&su, imp->takes.data);

		imp->data.reset();
		imp->data.ptr = sf::impBoxAllocate(header.s_audio.uncompressed_size + 16, &sf::destructRangeImp<char>);
		imp->size = header.s_audio.uncompressed_size;
		spsound_decode_audio_to(&su, imp->data);
	}

	if (spfile_util_failed(&su.file)) {
		imp->assetFailLoading();
		spfile_util_free(&su.file);
		imp->data.reset();
		return;
	}

	for (spsound_take &take : imp->takes) {
		imp->info = header.info;
		if (take.format == SPSOUND_FORMAT_PCM16) {
			imp->sharedSources.push(sf::box<Pcm16Source>(imp->data, take.file_offset, take.sample_rate, take.num_channels, take.length_in_samples));
		} else {
			imp->sharedSources.push();
		}
	}


	imp->assetFinishLoading();
}

void SoundImp::assetStartLoading()
{
	sf::SmallStringBuf<256> assetName;
	assetName.append(name, ".spsnd");

	ContentFile::loadAsync(assetName, &loadImp, this);
}

void SoundImp::assetUnload()
{
	data.reset();
	sharedSources.clear();
}

sf::Box<AudioSource> Sound::getSource(uint32_t takeIndex) const
{
	SoundImp *imp = (SoundImp*)this;
	if (imp->sharedSources[takeIndex]) {
		return imp->sharedSources[takeIndex];
	}

	const spsound_take &take = takes[takeIndex];
	if (take.format == SPSOUND_FORMAT_VORBIS) {
		return sf::box<VorbisSource>(imp->data, take.file_offset, take.file_size, take.temp_memory_required,
			take.sample_rate, take.num_channels);
	} else {
		sf_failf("Unhandled non-shared source: %u", take.format);
		return { };
	}
}

}
