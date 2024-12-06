#include "AudioSourceBase.h"
#include <al.h>
#include "Core/ServiceLocator.h"
#include "AudioManager.h"

Spear::AudioSourceBase::AudioSourceBase()
{
	Spear::ServiceLocator::GetAudioManager().RegisterAudioSource(this);

	alGenSources(1, &m_source);
	RefreshSourceSettings();
	alSourcei(m_source, AL_BUFFER, 0);
	ASSERT(!alGetError());
}

Spear::AudioSourceBase::~AudioSourceBase()
{
	Spear::ServiceLocator::GetAudioManager().DeregisterAudioSource(this);

	if (m_source != 0)
	{
		alDeleteSources(1, &m_source);
		ASSERT(!alGetError());
	}
}

void Spear::AudioSourceBase::SetPosition(const Vector3f& position)
{
	alSource3f(m_source, AL_POSITION, position.x, position.y, position.z);
	m_position = position;
}

void Spear::AudioSourceBase::SetVelocity(const Vector3f& velocity)
{
	alSource3f(m_source, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
	m_velocity = velocity;
}

void Spear::AudioSourceBase::SetPitch(float pitch)
{
	alSourcef(m_source, AL_PITCH, m_pitch);
	m_pitch = pitch;
}

void Spear::AudioSourceBase::SetGain(float gain)
{
	alSourcef(m_source, AL_GAIN, m_gain);
	m_gain = gain;
}

void Spear::AudioSourceBase::SetLooping(bool looping)
{
	alSourcei(m_source, AL_LOOPING, m_looping);
	m_looping = looping;
}

void Spear::AudioSourceBase::RefreshSourceSettings()
{
	alSource3f(m_source, AL_POSITION, m_position.x, m_position.y, m_position.z);
	alSource3f(m_source, AL_VELOCITY, m_velocity.x, m_velocity.y, m_velocity.z);
	alSourcef(m_source, AL_PITCH, m_pitch);
	alSourcef(m_source, AL_GAIN, m_gain);
	alSourcei(m_source, AL_LOOPING, m_looping);
}
