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
	enum Flag
	{
		Stop = 0x1,
	};

	SoundInstance *next = nullptr;
	sf::Box<sp::AudioSource> source;
	AudioInfo info;
	sp::AudioSampler sampler;
	uint32_t atomicFlags = 0;

	uint32_t spatialSoundIndex = ~0u;
	uint32_t trackedSoundId = ~0u;

	SoundOpts optsBuf[4] = { };
	uint32_t optsIndex = 0;

	void setFlag(uint32_t flags) { mxa_or32_nf(&atomicFlags, flags); }
	uint32_t getFlags() const { return mxa_load32_nf(&atomicFlags); }

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

			uint32_t flags = inst->getFlags();

			SoundOpts soundOpts = inst->getOpts();

			sp::AudioMixOpts opts;
			opts.sampleRate = sampleRate;
			opts.loop = inst->info.loop;
			opts.pitch = inst->info.pitch;
			opts.volumeNext[0] = soundOpts.volume[0];
			opts.volumeNext[1] = soundOpts.volume[1];

			if (flags & SoundInstance::Stop) {
				opts.volumeNext[0] = 0.0f;
				opts.volumeNext[1] = 0.0f;
			}

			inst->sampler.advanceMixStereo(dstBuf, numSamples, inst->source, opts);

			bool ended = inst->sampler.ended;

			if (flags & SoundInstance::Stop) {
				if (inst->sampler.volumeSrc[0] + inst->sampler.volumeSrc[1] <= 0.00001f) {
					ended = true;
				}
			}

			if (ended) {
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
	struct PendingSound
	{
		sp::SoundRef sound;
		AudioInfo info;
		uint32_t entitySoundId = ~0u;
		float delay = 0.0f;
	};

	struct SoundComponentData
	{
		sf::Array<sp::SoundRef> refs;
	};

	struct SoundEffectData
	{
		sp::SoundRef ref;
	};

	struct EntitySound
	{
		sf::Vec3 offset;
		SoundInstance *instance = nullptr;
		uint32_t pendingSoundId = ~0u;
	};

	sf::Array<SoundInstance> soundInstances;
	SoundInstance *nextFreeInstance = nullptr;

	sf::Array<SoundInstance*> playingSpatialSounds;

	sf::Array<PendingSound> pendingSounds;

	sf::Array<EntitySound> entitySounds;
	sf::Array<uint32_t> freeEntitySoundIds;

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

		if (info.positional) {
			sf::Vec3 pos = info.position - viewToWorld.cols[3];
			float x = sf::dot(pos, viewToWorld.cols[0]);
			float dist = sf::length(pos);
			float cosTheta = x / (dist + 1.0f);
			float sinHalfTheta = sf::sqrt(sf::max(0.0f, 1.0f - cosTheta) * 0.5f);
			float cosHalfTheta = sf::sqrt(sf::max(0.0f, 1.0f - sinHalfTheta*sinHalfTheta));

			float volume = 1.0f / (1.0f + dist * 0.1f);
			opts.volume[0] = sinHalfTheta * info.volume * volume;
			opts.volume[1] = cosHalfTheta * info.volume * volume;
		} else {
			opts.volume[0] = info.volume;
			opts.volume[1] = info.volume;
		}

		return opts;
	}

	void playLoadedSoundImp(const sp::SoundRef &ref, const AudioInfo &info, uint32_t entitySoundId)
	{
		if (!ref.isLoaded()) return;
		SoundInstance *inst = allocInstance();
		if (!inst) return;

		inst->source = ref->getSource();
		inst->info = info;

		if (entitySoundId != ~0u) {
			entitySounds[entitySoundId].instance = inst;
		}

		if (info.positional) {
			inst->spatialSoundIndex = playingSpatialSounds.size;
			playingSpatialSounds.push(inst);
		}

		inst->setOpts(evaluateSoundOpts(inst->info));

		g_audioThread.pushInstance(&g_audioThread.incoming, inst);
	}

	void playSoundImp(const sp::SoundRef &sound, const AudioInfo &info, uint32_t entitySoundId, float delay=0.0f)
	{
		if (sound.isLoading() || delay > 0.0f) {
			if (entitySoundId != ~0u) {
				entitySounds[entitySoundId].pendingSoundId = pendingSounds.size;
			}
			pendingSounds.push({ sound, info, entitySoundId, delay });
		} else {
			playLoadedSoundImp(sound, info, entitySoundId);
		}
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

	sf::Box<void> preloadSound(const sv::SoundEffect &effect) override
	{
		auto data = sf::box<SoundEffectData>();
		data->ref.load(effect.soundName);
		return data;
	}

	void playOneShot(const sp::SoundRef &sound, const AudioInfo &info) override
	{
		playSoundImp(sound, info, ~0u);
	}

	void playOneShot(const sv::SoundEffect &effect, const sf::Vec3 &position, float delay) override
	{
		if (!effect.soundName) return;

		AudioInfo info = { };
		info.position = position;
		info.volume = effect.volume;
		info.pitch = effect.pitch + effect.pitchVariance * rng.nextFloat();
		sp::SoundRef sound { effect.soundName };
		playSoundImp(sound, info, ~0u, delay);
	}

	void addSound(Systems &systems, uint32_t entityId, uint8_t componentIndex, const sv::SoundComponent &c, const Transform &transform) override
	{
		if (c.sounds.size == 0) return;
		sp::SoundRef sound { c.sounds[rng.nextU32() % c.sounds.size].assetName };

		uint32_t entitySoundId = entitySounds.size;
		if (freeEntitySoundIds.size > 0) {
			entitySoundId = freeEntitySoundIds.popValue();
		} else {
			entitySounds.push();
		}

		EntitySound &entitySound = entitySounds[entitySoundId];
		entitySound.offset = c.offset;

		AudioInfo info;
		info.position = transform.transformPoint(entitySound.offset);
		info.volume = c.volume + c.volumeVariance * rng.nextFloat();
		info.pitch = c.pitch + c.pitchVariance * rng.nextFloat();
		info.loop = c.loop;
		playSoundImp(sound, info, entitySoundId);

		systems.entities.addComponent(entityId, this, entitySoundId, 0, componentIndex, Entity::UpdateTransform);
	}

	void updateTransform(Systems &systems, uint32_t entityId, const EntityComponent &ec, const TransformUpdate &update) override
	{
		uint32_t entitySoundId = ec.userId;
		EntitySound &entitySound = entitySounds[entitySoundId];

		sf::Vec3 pos = sf::transformPoint(update.entityToWorld, entitySound.offset);
		if (entitySound.instance) {
			entitySound.instance->info.position = pos;
		}
		if (entitySound.pendingSoundId != ~0u) {
			pendingSounds[entitySound.pendingSoundId].info.position = pos;
		}
	}

	void remove(Systems &systems, uint32_t entityId, const EntityComponent &ec) override
	{
		uint32_t entitySoundId = ec.userId;
		EntitySound &entitySound = entitySounds[entitySoundId];

		if (entitySound.instance) {
			if (entitySound.instance->info.loop) {
				entitySound.instance->setFlag(SoundInstance::Stop);
			}
		}

		uint32_t pendingSoundId = entitySound.pendingSoundId;
		if (pendingSoundId != ~0u) {
			PendingSound &p = pendingSounds[pendingSoundId];
			if (p.info.loop) {
				uint32_t backEntitySoundId = pendingSounds.back().entitySoundId;
				if (backEntitySoundId != ~0u) {
					entitySounds[backEntitySoundId].pendingSoundId = pendingSoundId;
				}
				pendingSounds.removeSwap(pendingSoundId);
			} else {
				p.entitySoundId = ~0u;
			}
		}

		freeEntitySoundIds.push(entitySoundId);
		sf::reset(entitySound);
	}

	void updateSpatialSounds(const sf::Mat34 &worldToView) override
	{
		viewToWorld = sf::inverse(worldToView);

		for (SoundInstance *inst : playingSpatialSounds) {
			inst->setOpts(evaluateSoundOpts(inst->info));
		}
	}

	void update(const FrameArgs &frameArgs) override
	{
		float dt = frameArgs.dt;

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
		for (uint32_t i = 0; i < pendingSounds.size; i++) {
			PendingSound &p = pendingSounds[i];
			if (p.sound.isLoading()) continue;
			p.delay -= dt;
			if (p.delay > 0.0f) continue;

			playLoadedSoundImp(p.sound, p.info, p.entitySoundId);

			uint32_t backEntitySoundId = pendingSounds.back().entitySoundId;
			if (backEntitySoundId != ~0u) {
				entitySounds[backEntitySoundId].pendingSoundId = i;
			}
			pendingSounds.removeSwap(i--);
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

