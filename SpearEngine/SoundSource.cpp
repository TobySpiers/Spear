#include "SoundSource.h"
#include "ServiceLocator.h"
#include "AudioManager.h"
#include "al.h"

Spear::SoundSource::SoundSource()
{
	alGenSources(1, &m_source);
	alSourcef(m_source, AL_PITCH, m_pitch);
	alSourcef(m_source, AL_GAIN, m_gain);
	alSource3f(m_source, AL_POSITION, m_position[0], m_position[1], m_position[2]);
	alSource3f(m_source, AL_VELOCITY, m_velocity[0], m_velocity[1], m_velocity[2]);
	alSourcei(m_source, AL_LOOPING, m_looping);
	alSourcei(m_source, AL_BUFFER, 0);
	ASSERT(!alGetError());
}

Spear::SoundSource::~SoundSource()
{
	alDeleteSources(1, &m_source);
	ASSERT(!alGetError());
}

void Spear::SoundSource::PlaySound(int soundId)
{
	// If sound has changed, updated buffer registered with source
	if (soundId != -1 && m_curSoundId != soundId)
	{
		static AudioManager& audio = ServiceLocator::GetAudioManager();
		alSourcei(m_source, AL_BUFFER, audio.GetBufferForId(soundId));
		m_curSoundId = soundId;
		ASSERT(!alGetError());
	}

	// Validate and play sound
	ASSERT(m_curSoundId != -1); // no sound was ever assigned!
	ASSERT(m_pitch > 0);
	alSourcePlay(m_source);
}

void Spear::SoundSource::StopSound()
{
	alSourceStop(m_source);
}
