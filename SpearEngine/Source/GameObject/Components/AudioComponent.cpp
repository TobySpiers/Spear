#include "AudioComponent.h"

void AudioComponent::SetSound(int soundId)
{
	m_soundSource.SetSound(soundId);
}

void AudioComponent::Play()
{
	m_soundSource.Play();
}

void AudioComponent::Stop()
{
	m_soundSource.Stop();
}