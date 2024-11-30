#include "AudioEmitter.h"
#include "ServiceLocator.h"
#include "AudioManager.h"

GAMEOBJECT_REGISTER(AudioEmitter)

void AudioEmitter::OnCreated()
{
	if (m_bPlayOnStart)
	{
		m_soundSource.Play();
	}
}
