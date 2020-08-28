#include "AudioSystem.h"

#include "client/AreaSystem.h"

#include "game/DebugDraw.h"

#include "sf/Geometry.h"
#include "sf/Random.h"

#include "sp/Audio.h"
#include "sp/Sound.h"

#include "sf/ext/mx/mx_platform.h"

#include "sf/Mutex.h"

namespace cl {

const uint32_t MaxSoundInstances = 128;

struct SoundOpts
{
	float volume[2];
};

struct SoundInstance
{
	SoundInstance *next = nullptr;
	sf::Box<sp::AudioSource> source;
	AudioInfo info;
	sp::AudioSampler sampler;
	uint32_t spatialSoundIndex = ~0u;
	uint32_t trackedSoundId = ~0u;

	SoundOpts optsBuf[4] = { };
	uint32_t optsIndex = 0;

	SoundOpts getOpts() const { return optsBuf[mxa_load32_acq(&optsIndex) & 3]; }
	void setOpts(const SoundOpts &opts) {
		uint32_t ix = mxa_load32_nf(&optsIndex);
		optsBuf[(ix + 1) & 3] = opts;
		mxa_inc32_rel(&optsIndex);
	}
};

struct AudioThread
{
	static const uint32_t ShutdownSamples = 128;

	SoundInstance *incoming = nullptr;
	SoundInstance *outgoing = nullptr;

	sf::Array<SoundInstance*> instances;
	sf::Array<float> shutdownBuffer;

	sp::AudioLimiter limiter;

	sf::Mutex mutex;
	uint32_t prevSampleRate = 0;

	AudioThread()
	{
		instances.reserve(MaxSoundInstances);
		shutdownBuffer.reserve(ShutdownSamples * 2);
	}

	void pushInstance(SoundInstance **list, SoundInstance *instance) {
		SoundInstance *last = instance;
		while (last->next) last = last->next;

		SoundInstance *next;
		do {
			next = (SoundInstance*)mxa_load_ptr(list);
			last->next = next;
		} while (!mxa_cas_ptr(list, next, instance));
	}

	SoundInstance *popInstances(SoundInstance **list)
	{
		return (SoundInstance*)mxa_exchange_ptr(list, nullptr);
	}

	void renderAudio(float *finalBuffer, uint32_t numFinalSamples, uint32_t sampleRate)
	{
		// Add new sounds
		{
			SoundInstance *inst = popInstances(&incoming);
			while (inst) {
				instances.push(inst);
				inst = inst->next;
			}
		}

		SoundInstance *nextOut = nullptr;

		uint32_t numSamples;
		float *dstBuf = limiter.getStereoBuffer(numSamples, numFinalSamples, sampleRate);
		memset(dstBuf, 0, sizeof(float) * numSamples * 2);

		// Play sounds
		for (uint32_t i = 0; i < instances.size; i++) {
			SoundInstance *inst = instances[i];

			SoundOpts soundOpts = inst->getOpts();

			sp::AudioMixOpts opts;
			opts.sampleRate = sampleRate;
			opts.loop = inst->info.loop;
			opts.pitch = inst->info.pitch;
			opts.volumeNext[0] = soundOpts.volume[0];
			opts.volumeNext[1] = soundOpts.volume[1];

			inst->sampler.advanceMixStereo(dstBuf, numSamples, inst->source, opts);

			if (inst->sampler.ended) {
				inst->next = nextOut;
				nextOut = inst;
				instances.removeSwap(i--);
			}
		}

		limiter.limitStereo(finalBuffer, numFinalSamples, sampleRate);

		// Update output
		if (nextOut) {
			pushInstance(&outgoing, nextOut);
		}
	}

	void pullAudio(float *dstBuf, uint32_t numSamples, uint32_t sampleRate)
	{
		sf::MutexGuard mg(mutex);
		prevSampleRate = sampleRate;

		if (shutdownBuffer.size > 0) {
			uint32_t numShutdown = sf::min(shutdownBuffer.size / 2, numSamples);

			memcpy(dstBuf, shutdownBuffer.data, numShutdown * 2 * sizeof(float));
			shutdownBuffer.removeOrdered(0, numShutdown * 2);

			numSamples -= numShutdown;
			dstBuf += numShutdown * 2;
		}

		if (numSamples > 0) {
			renderAudio(dstBuf, numSamples, sampleRate);
		}
	}

	void shutdown()
	{
		sf::MutexGuard mg(mutex);
		if (prevSampleRate == 0) return;

		shutdownBuffer.clear();
		shutdownBuffer.resize(ShutdownSamples * 2);
		renderAudio(shutdownBuffer.data, ShutdownSamples, prevSampleRate);

		float v = 1.0f, dv = -1.0f / (float)ShutdownSamples;
		float *dst = shutdownBuffer.data;
		for (uint32_t i = 0; i < ShutdownSamples; i++) {
			dst[0] *= v;
			dst[1] *= v;
			v += dv;
			dst += 2;
		}

		limiter.reset();
		instances.clear();
		outgoing = nullptr;
	}

	bool isFinished()
	{
		sf::MutexGuard mg(mutex);
		return shutdownBuffer.size == 0 && instances.size == 0;
	}
};

AudioThread g_audioThread;

struct AudioSystemImp final : AudioSystem
{
	struct PendingOneShot
	{
		sp::SoundRef sound;
		AudioInfo info;
	};

	struct TrackedSound
	{
		sp::SoundRef sound;
		SoundInstance *instance;
	};

	struct SoundComponentData
	{
		sf::Array<sp::SoundRef> refs;
	};

	sf::Array<SoundInstance> soundInstances;
	SoundInstance *nextFreeInstance = nullptr;

	sf::Array<SoundInstance*> playingSpatialSounds;

	sf::Array<PendingOneShot> pendingOneShots;

	sf::Random rng;

	sf::Mat34 viewToWorld;

	SoundInstance *allocInstance()
	{
		SoundInstance *inst = nextFreeInstance;
		if (inst) {
			nextFreeInstance = inst->next;
			inst->next = nullptr;
		}
		return inst;
	}

	SoundOpts evaluateSoundOpts(const AudioInfo &info)
	{
		SoundOpts opts;

		sf::Vec3 pos = info.position - viewToWorld.cols[3];
		float x = sf::dot(pos, viewToWorld.cols[0]);
		float dist = sf::length(pos);
		float cosTheta = x / (dist + 1.0f);
		float sinHalfTheta = sf::sqrt(sf::max(0.0f, 1.0f - cosTheta) * 0.5f);
		float cosHalfTheta = sf::sqrt(sf::max(0.0f, 1.0f - sinHalfTheta*sinHalfTheta));

		float volume = 1.0f / (1.0f + dist * 0.1f);
		opts.volume[0] = sinHalfTheta * info.volume * volume;
		opts.volume[1] = cosHalfTheta * info.volume * volume;

		return opts;
	}

	void playOneShotImp(const sp::SoundRef &ref, const AudioInfo &info)
	{
		if (!ref.isLoaded()) return;
		SoundInstance *inst = allocInstance();
		if (!inst) return;

		inst->source = ref->getSource();
		inst->info = info;

		inst->spatialSoundIndex = playingSpatialSounds.size;
		playingSpatialSounds.push(inst);

		g_audioThread.pushInstance(&g_audioThread.incoming, inst);
	}

	// API

	AudioSystemImp(const SystemsDesc &desc)
	{
		rng = sf::Random(desc.seed[2], 23);

		soundInstances.resize(MaxSoundInstances);
		for (uint32_t i = 0; i < MaxSoundInstances - 1; i++) {
			soundInstances[i].next = &soundInstances[i + 1];
		}
		nextFreeInstance = &soundInstances[0];
	}

	~AudioSystemImp()
	{
		g_audioThread.shutdown();
	}

	sf::Box<void> preloadSound(const sv::SoundComponent &c) override
	{
		auto data = sf::box<SoundComponentData>();
		data->refs.reserve(c.sounds.size);
		for (const sv::SoundInfo &info : c.sounds) {
			data->refs.push().load(info.assetName);
		}
		return data;
	}

	void playOneShot(const sp::SoundRef &sound, const AudioInfo &info) override
	{
		if (sound.isLoading()) {
			pendingOneShots.push({ sound, info });
		} else {
			playOneShotImp(sound, info);
		}
	}

	void addSound(Systems &systems, uint32_t entityId, uint8_t componentIndex, const sv::SoundComponent &c, const Transform &transform) override
	{
		if (c.sounds.size == 0) return;
		sp::SoundRef sound { c.sounds[rng.nextU32() % c.sounds.size].assetName };

		AudioInfo info;
		info.position = transform.position;
		info.volume = c.volume + c.volumeVariance * rng.nextFloat();
		info.pitch = c.pitch + c.pitchVariance * rng.nextFloat();
		info.loop = c.loop;
		playOneShot(sound, info);
	}

	void updateTransform(Systems &systems, uint32_t entityId, const EntityComponent &ec, const TransformUpdate &update) override
	{
	}

	void remove(Systems &systems, uint32_t entityId, const EntityComponent &ec) override
	{
	}

	void updateSpatialSounds(const sf::Mat34 &worldToView) override
	{
		viewToWorld = sf::inverse(worldToView);

		for (SoundInstance *inst : playingSpatialSounds) {
			inst->setOpts(evaluateSoundOpts(inst->info));
		}
	}

	void update() override
	{
		// Recycle sound instances
		{
			SoundInstance *inst = g_audioThread.popInstances(&g_audioThread.outgoing);
			while (inst) {
				SoundInstance *next = inst->next;

				uint32_t spatialIx = inst->spatialSoundIndex;
				playingSpatialSounds.back()->spatialSoundIndex = spatialIx;
				playingSpatialSounds.removeSwap(spatialIx);

				sf::reset(*inst);
				inst->next = nextFreeInstance;
				nextFreeInstance = inst;

				inst = next;
			}
		}

		// Update pending one shots
		for (uint32_t i = 0; i < pendingOneShots.size; i++) {
			PendingOneShot &p = pendingOneShots[i];
			if (p.sound.isLoading()) continue;
			playOneShotImp(p.sound, p.info);
			pendingOneShots.removeSwap(i--);
		}
	}


};

sf::Box<AudioSystem> AudioSystem::create(const SystemsDesc &desc) { return sf::box<AudioSystemImp>(desc); }

void pullAudioStereo(float *dstBuf, uint32_t numSamples, uint32_t sampleRate)
{
	g_audioThread.pullAudio(dstBuf, numSamples, sampleRate);
}

bool isAudioFinished()
{
	return g_audioThread.isFinished();
}

}

