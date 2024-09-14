#pragma once
#include "Core.h"
#include "SoundSource.h"
#include "StreamSource.h"
#include <unordered_set>

typedef struct ALCdevice;
typedef struct ALCcontext;

namespace Spear
{
	class AudioManager
	{
		NO_COPY(AudioManager);
		friend Core;
		friend StreamSource;
		friend SoundSource;

	public:
		AudioManager();
		~AudioManager();

		static AudioManager& Get();

		void InitSoundsFromFolder(const char* dir);
		void ReleaseSounds();

		// TOOD: Extend to support multiple simultaneous global sounds/streams
		// Global sound effects (non-positional)
		void GlobalPlaySound(int soundId);
		void GlobalStopSound();
		// Global sound streaming (non-positional)
		void GlobalPlayStream(const char* filepath);
		void GlobalStopStream();

		// Stops global SoundSource and all StreamSources
		// TODO: Does not stop non-global SoundSources, need to register these (similar to StreamSource setup) and include global controls such as stop/volume/pitch/etc.
		void StopAllAudio();

	private:
		void RegisterActiveStream(StreamSource& stream);
		void UpdateStreamingSounds();

		bool LoadSound(const char* filepath, int slot);
		u32 GetBufferForId(int soundId) const;

		ALCdevice* m_device{ nullptr };
		ALCcontext* m_context{ nullptr };
		std::unordered_map<int, u32> m_audioBufferSlots;

		// Dynamically allocated - self needs to be constructed (openAL initialised) prior to constructing sources
		SoundSource* m_pGlobalSoundSource{ nullptr };
		StreamSource* m_pGlobalStreamSource{ nullptr };

		// For tracking & updating active audio streams
		std::unordered_set<StreamSource*> m_activeStreams;
	};
}