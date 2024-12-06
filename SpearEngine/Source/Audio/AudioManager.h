#pragma once
#include "Core/Core.h"
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
		friend AudioSourceBase;
		friend StreamSource;
		friend SoundSource;

	public:
		AudioManager();
		~AudioManager();
		void OnCreated();

		static AudioManager& Get();

		// Loads sounds associated with SoundIDs (streaming files are NOT loaded here)
		void InitSoundsFromFolder(const char* dir);

		// Global sound effects (non-positional)
		void GlobalPlaySound(int soundId);
		void GlobalStopSound();

		// Global sound streaming (non-positional)
		void GlobalPlayStream(const char* filepath);
		void GlobalStopStream();

		// Stops all AudioSources from playing
		void StopAllAudio();

	private:
		void RegisterAudioSource(AudioSourceBase* audioSource);
		void DeregisterAudioSource(AudioSourceBase* audioSource);

		void AddToPlayingStreams(StreamSource& stream);
		void UpdatePlayingStreams();
		std::vector<StreamSource*> m_playingStreams;

		bool LoadSound(const char* filepath, int slot);
		u32 GetBufferForSoundId(int soundId) const;
		void ReleaseSounds();

		// For tracking & updating sources
		SoundSource* m_soundSource = nullptr;
		StreamSource* m_streamSource = nullptr;
		std::unordered_set<AudioSourceBase*> m_audioSources;

		ALCdevice* m_device{ nullptr };
		ALCcontext* m_context{ nullptr };
		std::unordered_map<int, u32> m_audioBufferSlots;
	};
}