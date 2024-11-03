#include "AudioEmitter.h"

GAMEOBJECT_REGISTER(AudioEmitter)

void AudioEmitter::OnCreated()
{
	m_soundSource.Init();
	m_soundSource.SetPosition(m_position);
	m_soundSource.SetVelocity(m_velocity);
	if (m_bPlayOnStart)
	{
		m_soundSource.PlaySound();
	}
}

void AudioEmitter::OnDestroy()
{
	m_soundSource.Destroy();
}