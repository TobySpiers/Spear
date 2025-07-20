#include "AudioEmitter.h"
#include "Core/ServiceLocator.h"
#include "Audio/AudioManager.h"

GAMEOBJECT_REGISTER(AudioEmitter)

AudioEmitter::AudioEmitter()
{
	m_audioComponent = AddComponent<AudioComponent>();
}

void AudioEmitter::OnCreated()
{
	if (m_bPlayOnStart)
	{
		m_audioComponent->Play();
	}
}
