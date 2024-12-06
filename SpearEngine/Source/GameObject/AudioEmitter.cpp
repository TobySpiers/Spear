#include "AudioEmitter.h"
#include "Core/ServiceLocator.h"
#include "Audio/AudioManager.h"

GAMEOBJECT_REGISTER(AudioEmitter)

void AudioEmitter::OnCreated()
{
	if (m_bPlayOnStart)
	{
		m_soundSource.Play();
	}
}
