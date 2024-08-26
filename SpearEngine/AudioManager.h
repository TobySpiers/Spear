#pragma once
#include "Core.h"
#include "SDL_mixer.h"

namespace Spear
{
	constexpr int MUSIC_MAX{ 4 };
	constexpr int SOUNDS_MAX{ 32 };

	class AudioManager
	{
		NO_COPY(AudioManager);

	public:
		AudioManager();
		~AudioManager();

		void LoadMusic(const char* dir);
		void LoadSounds(const char* dir);

		// PlayMusic. Defaults to -1 loops (endless).
		void PlayMusic(int slot, int loops = -1);
		void PauseMusic();
		void ResumeMusic();
		void StopMusic();
		void FadeOutMusic(int fadeMs = 1000);

		void PlaySound(int slot);

		void UnloadMusic();
		void UnloadSounds();

	private:
		int m_curMusic{ -1 };
		Mix_Music* m_loadedMusic[MUSIC_MAX] = {};
		Mix_Chunk* m_loadedSounds[SOUNDS_MAX] = {};
	};
}