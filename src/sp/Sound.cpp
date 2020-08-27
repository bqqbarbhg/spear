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

	void *data = nullptr;
	size_t size = 0;

	~SoundImp()
	{
		if (data) {
			sf::impBoxDecRef(data);
			data = nullptr;
		}
	}
};

struct VorbisSource : AudioSource
{
	void *dataRef = nullptr;
	stb_vorbis *vorbis = nullptr;
	char tempMemory[200u * 1000u];

	VorbisSource(void *data, size_t size)
	{
		if (!data || !size) return;

		sf::impBoxIncRef(data);

		// Fixed 200kB buffer per stream

		stb_vorbis_alloc alloc;
		alloc.alloc_buffer = tempMemory;
		alloc.alloc_buffer_length_in_bytes = (int)sizeof(tempMemory);

		int error = 0;
		vorbis = stb_vorbis_open_memory((const unsigned char*)data, (int)size, &error, &alloc);

		stb_vorbis_info info = stb_vorbis_get_info(vorbis);
		sampleRate = info.sample_rate;
		numChannels = sf::clamp(info.channels, 1, 2);
	}

	~VorbisSource()
	{
		if (dataRef) {
			sf::impBoxDecRef(dataRef);
			dataRef = nullptr;
		}
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

#if 0
struct WavPlayer : Sound::Player
{
	void *dataRef = nullptr;
	uint32_t dataSamples = 0;
	uint32_t cursor = 0;

	WavPlayer(void *data, uint32_t numChannels, uint32_t numSamples, uint32_t sampleRate)
	{
		this->sampleRate = sampleRate;
		if (!data) return;
		sf::impBoxIncRef(data);
	}

	~WavPlayer()
	{
		if (dataRef) {
			sf::impBoxDecRef(dataRef);
			dataRef = nullptr;
		}
	}

	virtual uint32_t decodeMono(float *chan, uint32_t numSamples, bool loop) override
	{
		const int16_t *samples = (const int16_t*)dataRef;

		sf::ScalarFloat4 scale = 1.0f/32768.0f;

		uint32_t numSafe = sf::min(dataSamples - cursor, numSamples) & 0x7;
		uint32_t num = 0;
		const int16_t *safeSamples = samples + cursor;
		float *safeChan = chan;
		while (num < numSafe) {
			sf::Float4 a, b;
			sf::Float4::load8xi16(a, b, safeSamples);
			a *= scale;
			b *= scale;
			a.storeu(safeChan + 0);
			a.storeu(safeChan + 4);

			safeChan += 8;
			safeSamples += 8;
		}
		cursor += numSafe;

		while (num < numSamples) {
			if (cursor >= dataSamples) {
				if (loop) {
					cursor = 0;
				} else {
					break;
				}
			}

			chan[num++] = (float)samples[cursor++];
		}

		if (num < numSamples) {
			memset(chan + num, 0, (numSamples - num) * sizeof(float));
		}

		return num;
	}

	virtual uint32_t decodeStereo(float *left, float *right, uint32_t numSamples, bool loop) override
	{
		// TODO
	}
};
#endif

AssetType Sound::SelfType = { "Sound", sizeof(SoundImp), sizeof(Sound::PropType),
	[](Asset *a) { new ((SoundImp*)a) SoundImp(); }
};

static void loadImp(void *user, const ContentFile &file)
{
	SoundImp *imp = (SoundImp*)user;

	if (file.size == 0) {
		imp->assetFailLoading();
	}

	imp->data = sf::impBoxAllocate(file.size, &sf::destructRangeImp<char>);
	imp->size = file.size;
	memcpy(imp->data, file.data, file.size);

	imp->assetFinishLoading();
}

void SoundImp::assetStartLoading()
{
	ContentFile::loadAsync(name, &loadImp, this);
}

void SoundImp::assetUnload()
{
	if (data) {
		sf::impBoxDecRef(data);
		data = nullptr;
	}
}

sf::Box<AudioSource> Sound::getSource() const
{
	SoundImp *imp = (SoundImp*)this;
	return sf::box<VorbisSource>(imp->data, imp->size);
}

}
