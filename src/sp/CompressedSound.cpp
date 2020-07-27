#include "CompressedSound.h"

#include "ContentFile.h"
#include "sf/HashMap.h"
#include "sf/Array.h"

#include "ext/stb/stb_vorbis.h"

namespace sp {

struct CompressedSoundImp : CompressedSound
{
	virtual void assetStartLoading() final;
	virtual void assetUnload() final;

	void *data = nullptr;
	stb_vorbis *vorbis = nullptr;
	sf::Array<char> tempMemory;
};

AssetType CompressedSound::SelfType = { "CompressedSound", sizeof(CompressedSoundImp), sizeof(CompressedSound::PropType),
	[](Asset *a) { new ((CompressedSoundImp*)a) CompressedSoundImp(); }
};

static void loadImp(void *user, const ContentFile &file)
{
	CompressedSoundImp *imp = (CompressedSoundImp*)user;

	if (file.size > 0) {
		imp->data = sf::memAlloc(file.size);
		memcpy(imp->data, file.data, file.size);

		// Fixed 200kB buffer per stream
		imp->tempMemory.resizeUninit(200u * 1000u);

		stb_vorbis_alloc alloc;
		alloc.alloc_buffer = imp->tempMemory.data;
		alloc.alloc_buffer_length_in_bytes = (int)imp->tempMemory.size;

		int error = 0;
		imp->vorbis = stb_vorbis_open_memory((const unsigned char*)imp->data, file.size, &error, &alloc);
		if (imp->vorbis) {
			stb_vorbis_info info = stb_vorbis_get_info(imp->vorbis);
			imp->sampleRate = (uint32_t)info.sample_rate;
			imp->numChannels = (uint32_t)info.channels;

			// TODO: Pre-process these into a header?
			imp->numSamples = stb_vorbis_stream_length_in_samples(imp->vorbis);
			imp->lengthSeconds = stb_vorbis_stream_length_in_seconds(imp->vorbis);

			imp->assetFinishLoading();
			return;
		}
	}

	if (imp->data) {
		sf::memFree(imp->data);
		imp->data = nullptr;
	}
	imp->assetFailLoading();

}

void CompressedSoundImp::assetStartLoading()
{
	ContentFile::loadAsync(name, &loadImp, this);
}

void CompressedSoundImp::assetUnload()
{
	sf::reset(tempMemory);

	if (vorbis) {
		stb_vorbis_close(vorbis);
		vorbis = nullptr;
	}

	if (data) {
		sf::memFree(data);
		data = nullptr;
	}
}

uint32_t CompressedSound::decodeStereo(float *left, float *right, uint32_t numSamples)
{
	CompressedSoundImp *imp = (CompressedSoundImp*)this;

	float *buffers[] = { left, right };
	uint32_t num = stb_vorbis_get_samples_float(imp->vorbis, 2, buffers, (int)numSamples);

	if (num < numSamples) {
		memset(left + num, 0, (numSamples - num) * sizeof(float));
		memset(right + num, 0, (numSamples - num) * sizeof(float));
	}

	return num;
}

}
