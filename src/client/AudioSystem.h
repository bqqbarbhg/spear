#pragma once

#include "server/ServerState.h"

#include "client/System.h"

#include "sp/Sound.h"

namespace sf { struct FastRay; }

namespace cl {

struct AudioInfo
{
	sf::Vec3 position;
	float volume = 1.0f;
	float pitch = 1.0f;
	bool loop = false;
	bool follow = true;
	bool positional = true;
};

struct AudioSystem : EntitySystem
{
	static sf::Box<AudioSystem> create(const SystemsDesc &desc);

	virtual sf::Box<void> preloadSound(const sv::SoundComponent &c) = 0;
	virtual sf::Box<void> preloadSound(const sv::SoundEffect &effect) = 0;

	virtual void playOneShot(const sp::SoundRef &sound, const AudioInfo &info) = 0;
	virtual void playOneShot(const sv::SoundEffect &effect, const sf::Vec3 &position, float delay=0.0f) = 0;

	virtual uint32_t playAttached(Entities &entities, uint32_t entityId, const sv::SoundEffect &effect, const sf::Vec3 &offset, float delay=0.0f) = 0;
	virtual void removeAttached(Entities &entities, uint32_t attachedSoundId) = 0;

	virtual void addSound(Systems &systems, uint32_t entityId, uint8_t componentIndex, const sv::SoundComponent &c, const Transform &transform) = 0;

	virtual void updateSpatialSounds(const sf::Mat34 &worldToView) = 0;

	virtual void update(const FrameArgs &frameArgs) = 0;

};

void pullAudioStereo(float *dstBuf, uint32_t numSamples, uint32_t sampleRate);
bool isAudioFinished();

}
