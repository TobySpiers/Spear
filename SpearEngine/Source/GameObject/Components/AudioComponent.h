#pragma once
#include "GameObject/GameObjectComponent.h"
#include "Audio/SoundSource.h"

class AudioComponent : public GameObjectComponent
{
public:
	SERIALIZABLE_DERIVED(AudioComponent, GameObjectComponent, m_soundSource)

	void SetSound(int soundId);
	void Play();
	void Stop();

private:
	Spear::SoundSource m_soundSource;
};

