#include "SoundSource.h"
#include "ServiceLocator.h"
#include "AudioManager.h"
#include "al.h"

void Spear::SoundSource::Init()
{
	alGenSources(1, &m_source);
	alSourcef(m_source, AL_PITCH, m_pitch);
	alSourcef(m_source, AL_GAIN, m_gain);
	alSourcei(m_source, AL_LOOPING, m_looping);
	alSourcei(m_source, AL_BUFFER, 0);
	ASSERT(!alGetError());

	if (m_curSoundId != -1)
	{
		static AudioManager& audio = ServiceLocator::GetAudioManager();
		alSourcei(m_source, AL_BUFFER, audio.GetBufferForId(m_curSoundId));
	}
}

void Spear::SoundSource::SetPitch(float pitch)
{
	m_pitch = pitch;
	alSourcef(m_source, AL_PITCH, m_pitch);
}

void Spear::SoundSource::SetGain(float gain)
{
	m_gain = gain;
	alSourcef(m_source, AL_GAIN, m_gain);
}

void Spear::SoundSource::SetPosition(const Vector3f& position)
{
	alSource3f(m_source, AL_POSITION, position.x, position.y, position.z);
}

void Spear::SoundSource::SetVelocity(const Vector3f& velocity)
{
	alSource3f(m_source, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}

void Spear::SoundSource::SetLooping(bool looping)
{
	m_looping = looping;
	alSourcei(m_source, AL_LOOPING, m_looping);
}

void Spear::SoundSource::SetSound(int soundId)
{
	// If new sound, update OpenAL buffer
	ASSERT(m_source != -1);
	if (m_curSoundId != soundId)
	{
		static AudioManager& audio = ServiceLocator::GetAudioManager();
		alSourcei(m_source, AL_BUFFER, audio.GetBufferForId(soundId));
		m_curSoundId = soundId;
		ASSERT(!alGetError());
	}
}

Spear::SoundSource::~SoundSource()
{
	alDeleteSources(1, &m_source);
	ASSERT(!alGetError());
}

void Spear::SoundSource::PlaySound()
{
	if (m_curSoundId >= 0)
	{
		ASSERT(m_source != -1);
		alSourcePlay(m_source);
	}
}

void Spear::SoundSource::StopSound()
{
	alSourceStop(m_source);
}
