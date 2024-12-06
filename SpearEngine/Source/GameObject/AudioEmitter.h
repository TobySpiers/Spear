#pragma once
#include "GameObject.h"
#include "Audio/SoundSource.h"

class AudioEmitter : public GameObject
{
	GAMEOBJECT_SERIALISABLE(AudioEmitter, m_bPlayOnStart, m_soundSource)

public:
	virtual void OnCreated() override;

	bool m_bPlayOnStart{ false };
	Spear::SoundSource m_soundSource;
};