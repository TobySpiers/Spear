#include "SoundSource.h"
#include "ServiceLocator.h"
#include "AudioManager.h"
#include "al.h"

void Spear::SoundSource::PostDeserialize()
{
	RefreshSourceSettings();

	if (m_curSoundId != -1)
	{
		static AudioManager& audio = ServiceLocator::GetAudioManager();
		alSourcei(m_source, AL_BUFFER, audio.GetBufferForSoundId(m_curSoundId));
		AL_CATCH_ERROR();
	}
}

void Spear::SoundSource::SetSound(int soundId)
{
	// If new sound, update OpenAL buffer
	ASSERT(m_source != -1);
	if (m_curSoundId != soundId)
	{
		static AudioManager& audio = ServiceLocator::GetAudioManager();
		alSourcei(m_source, AL_BUFFER, audio.GetBufferForSoundId(soundId));
		m_curSoundId = soundId;
		AL_CATCH_ERROR();
	}
}

void Spear::SoundSource::Play()
{
	if (m_curSoundId >= 0)
	{
		ASSERT(m_source != -1);
		alSourcePlay(m_source);
	}
}

void Spear::SoundSource::Stop()
{
	alSourceStop(m_source);
}
