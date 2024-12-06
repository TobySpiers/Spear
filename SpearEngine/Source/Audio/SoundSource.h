#pragma once
#include "Core/Core.h"
#include "Core/Serializer.h"
#include "AudioSourceBase.h"

namespace Spear
{
	// Class for playing short sounds. Complete audio is stored in memory.
	class SoundSource : public AudioSourceBase
	{
	public:
		SERIALIZABLE_DERIVED(SoundSource, AudioSourceBase, m_curSoundId);
		virtual void PostDeserialize() override;

		void SetSound(int soundId);
		virtual void Play() override;
		virtual void Stop() override;

	private:
		int m_curSoundId{ -1 };
	};
}