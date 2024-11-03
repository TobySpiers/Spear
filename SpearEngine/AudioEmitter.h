#pragma once
#include "GameObject.h"
#include "SoundSource.h"

class AudioEmitter : public GameObject
{
	GAMEOBJECT_CLASS(AudioEmitter)
public:
	virtual void OnCreated() override;
	virtual void OnDestroy() override;

	Vector3f m_position{};
	Vector3f m_velocity{};
	bool m_bPlayOnStart{ false };
	Spear::SoundSource m_soundSource;
};
