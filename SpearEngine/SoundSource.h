#pragma once
#include "Core.h"

namespace Spear
{
	// Class for playing short sounds. Complete audio is stored in memory.
	class SoundSource
	{
	public:
		SoundSource();
		~SoundSource();

		// Plays sound at associated slot. Default arg '-1' will play last assigned slot.
		void PlaySound(int soundId = -1);
		void StopSound();

	private:
		// OpenAL
		u32 m_source{ 0 };
		int m_curSoundId{ -1 };

		// Source data
		float m_pitch{ 1.f };
		float m_gain{ 1.f };
		float m_position[3] = {0, 0, 0};
		float m_velocity[3] = {0, 0, 0};
		bool m_looping{ false };
	};
}