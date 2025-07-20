#pragma once
#include "GameObject.h"
#include "Audio/SoundSource.h"
#include "Components/AudioComponent.h"

class AudioEmitter : public GameObject
{
	GAMEOBJECT_DEFINITION_DATA(AudioEmitter, GameObject, m_bPlayOnStart)

public:
	AudioEmitter();
	virtual void OnCreated() override;

	bool m_bPlayOnStart{ false };

	AudioComponent* m_audioComponent{nullptr};
};