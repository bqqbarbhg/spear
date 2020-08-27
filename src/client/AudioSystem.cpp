#include "AudioSystem.h"

#include "client/AreaSystem.h"

#include "game/DebugDraw.h"

#include "sf/Geometry.h"
#include "sf/Random.h"

#include "sp/Audio.h"
#include "sp/Sound.h"

#include "sf/ext/mx/mx_platform.h"

namespace cl {

const uint32_t MaxSoundInstances = 128;

struct SoundInstance
{
	SoundInstance *next = nullptr;
	sf::Box<sp::AudioSource> source;
	AudioInfo info;
	sp::AudioSampler sampler;
	uint32_t trackedSoundId = ~0u;
	bool loop = false;
};

struct AudioThread
{
	SoundInstance *incoming = nullptr;
	SoundInstance *outgoing = nullptr;

	sf::Array<SoundInstance*> instances;

	uint32_t chunkSize = 512;

	AudioThread()
	{
		instances.reserve(MaxSoundInstances);
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

	void update(float *dstBuf, uint32_t numSamples, uint32_t sampleRate)
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

		// Play sounds
		for (uint32_t i = 0; i < instances.size; i++) {
			SoundInstance *inst = instances[i];

			sp::AudioMixOpts opts;
			opts.sampleRate = sampleRate;
			opts.loop = inst->loop;
			opts.pitch = inst->info.pitch;

			inst->sampler.advanceMixStereo(dstBuf, numSamples, inst->source, opts);

			if (inst->sampler.ended) {
				inst->next = nextOut;
				nextOut = inst;
				instances.removeSwap(i--);
			}
		}

		// Update output
		if (nextOut) {
			pushInstance(&outgoing, nextOut);
		}
	}
};

struct AudioSystemImp final : AudioSystem
{
	AudioThread thread;

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

	sf::Array<SoundInstance> soundInstances;
	SoundInstance *nextFreeInstance = nullptr;

	sf::Array<PendingOneShot> pendingOneShots;

	sf::Random rng;

	SoundInstance *allocInstance()
	{
		SoundInstance *inst = nextFreeInstance;
		if (inst) {
			nextFreeInstance = inst->next;
			inst->next = nullptr;
		}
		return inst;
	}

	void playOneShotImp(const sp::SoundRef &ref, const AudioInfo &info)
	{
		if (!ref.isLoaded()) return;
		SoundInstance *inst = allocInstance();
		if (!inst) return;

		inst->source = ref->getSource();
		inst->info = info;

		thread.pushInstance(&thread.incoming, inst);
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
		playOneShot(sound, info);
	}

	void updateTransform(Systems &systems, uint32_t entityId, const EntityComponent &ec, const TransformUpdate &update) override
	{
	}

	void remove(Systems &systems, uint32_t entityId, const EntityComponent &ec) override
	{
	}

	void update() override
	{
		// Recycle sound instances
		{
			SoundInstance *inst = thread.popInstances(&thread.outgoing);
			while (inst) {
				SoundInstance *next = inst;

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

	void audioThreadStereo(float *dstBuf, uint32_t numSamples, uint32_t sampleRate) override
	{
		thread.update(dstBuf, numSamples, sampleRate);
	}

};

sf::Box<AudioSystem> AudioSystem::create(const SystemsDesc &desc) { return sf::box<AudioSystemImp>(desc); }

}

