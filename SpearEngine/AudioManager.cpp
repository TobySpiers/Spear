#include "AudioManager.h"
#include "SDL_mixer.h"
#include "AssetManifest.h"

namespace Spear
{
	AudioManager::AudioManager()
	{

	}

	AudioManager::~AudioManager()
	{
		StopMusic();
		UnloadMusic();
		UnloadSounds();
	}

	void AudioManager::LoadMusic(const char* dir)
	{
		// Free up any previously loaded music
		UnloadMusic();

		// Load manifest of MP3 files in directory
		Manifest manifest;
		AssetManifest::GetManifest(dir, ".mp3", manifest);
		ASSERT(manifest.size() < MUSIC_MAX);

		for (int i = 0; i < manifest.size(); i++)
		{
			if (manifest[i] != "")
			{
				m_loadedMusic[i] = Mix_LoadMUS(manifest[i].c_str());
				if (!m_loadedMusic[i])
				{
					LOG(std::string("SDL Mixer failed to load ") + manifest[i] + "\nError: " + Mix_GetError());
				}
			}
		}
	}

	void AudioManager::LoadSounds(const char* dir)
	{
		// Free up any previously loaded sounds
		UnloadSounds();

		// Load manifest of WAV files in directory
		Manifest manifest;
		AssetManifest::GetManifest(dir, ".mp3", manifest);
		ASSERT(manifest.size() < SOUNDS_MAX);

		for (int i = 0; i < manifest.size(); i++)
		{
			if (manifest[i] != "")
			{
				m_loadedSounds[i] = Mix_LoadWAV(manifest[i].c_str());
				if (!m_loadedSounds[i])
				{
					LOG(std::string("SDL Mixer failed to load ") + manifest[i] + "\nError: " + Mix_GetError());
				}
			}
		}
	}

	void AudioManager::PlayMusic(int slot, int loops)
	{
		ASSERT(slot < MUSIC_MAX);
		if (Mix_Music* music = m_loadedMusic[slot])
		{
			Mix_PlayMusic(music, loops);
			m_curMusic = slot;
		}
	}

	void AudioManager::PauseMusic()
	{
		Mix_PauseMusic();
	}

	void AudioManager::ResumeMusic()
	{
		Mix_ResumeMusic();
	}

	void AudioManager::StopMusic()
	{
		Mix_HaltMusic();
	}

	void AudioManager::FadeOutMusic(int fadeMs)
	{
		Mix_FadeOutMusic(fadeMs);
	}

	void AudioManager::PlaySound(int slot)
	{
		ASSERT(slot < SOUNDS_MAX);
		if (Mix_Chunk* sound = m_loadedSounds[slot])
		{
			Mix_PlayChannel(-1, sound, 0);
		}
	}

	void AudioManager::UnloadMusic()
	{
		for (int i = 0; i < MUSIC_MAX; i++)
		{
			Mix_FreeMusic(m_loadedMusic[i]);
			m_loadedMusic[i] = nullptr;
		}
	}
	
	void AudioManager::UnloadSounds()
	{
		for (int i = 0; i < SOUNDS_MAX; i++)
		{
			Mix_FreeChunk(m_loadedSounds[i]);
			m_loadedSounds[i] = nullptr;
		}
	}
}