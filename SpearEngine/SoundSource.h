#pragma once
#include "Core.h"

namespace Spear
{
	// Class for playing short sounds. Complete audio is stored in memory.
	class SoundSource
	{
	public:
		~SoundSource();

		void Init();
		void SetPitch(float pitch);
		void SetGain(float gain);
		void SetPosition(const Vector3f& position);
		void SetVelocity(const Vector3f& velocity);
		void SetLooping(bool looping);
		void SetSound(int soundId);

		// Plays sound at associated slot. Default arg '-1' will play last assigned slot.
		void PlaySound();
		void StopSound();

	private:
		// OpenAL
		u32 m_source{ 0 };
		int m_curSoundId{ -1 };

		// Audio Settings
		float m_pitch{ 1.f };
		float m_gain{ 1.f };
		bool m_looping{ false };
	};
}