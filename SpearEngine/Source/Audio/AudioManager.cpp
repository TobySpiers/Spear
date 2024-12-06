#include "AudioManager.h"
#include "Core/ServiceLocator.h"
#include "Core/AssetManifest.h"
#include "AudioUtils.h"
#include "al.h"
#include "alc.h"
#include "alext.h"
#include "sndfile.hh"
#include <algorithm>

namespace Spear
{
	AudioManager::AudioManager()
	{
		m_device = alcOpenDevice(nullptr);
		if (!m_device)
		{
			LOG(std::string("OpenAl failed to intialize device"));
			return;
		}

		m_context = alcCreateContext(m_device, nullptr); // nullptr can be replaced with 'attributes' such as expected num of audio sources for performance
		if (alcMakeContextCurrent(m_context) != ALC_TRUE)
		{
			LOG(std::string("OpenAl failed to initialise context"));
		}

		const ALCchar* name{ nullptr };
		if (alcIsExtensionPresent(m_device, "ALC_ENUMERATE_ALL_EXT"))
		{
			name = alcGetString(m_device, ALC_ALL_DEVICES_SPECIFIER);
		}
		if (!name || alcGetError(m_device) != ALC_NO_ERROR)
		{
			name = alcGetString(m_device, ALC_DEVICE_SPECIFIER);
		}
		LOG(printf("OpenAL initialised with %s", name));

	}

	void AudioManager::OnCreated()
	{
		m_soundSource = new SoundSource;
		m_streamSource = new StreamSource;
	}

	AudioManager::~AudioManager()
	{
		StopAllAudio();

		delete m_soundSource;
		delete m_streamSource;
		m_soundSource = nullptr;
		m_streamSource = nullptr;

		ReleaseSounds();

		alcMakeContextCurrent(nullptr);
		alcDestroyContext(m_context);
		alcCloseDevice(m_device);
	}


	AudioManager& AudioManager::Get()
	{
		return ServiceLocator::GetAudioManager();
	}

	void AudioManager::RegisterAudioSource(AudioSourceBase* audioSource)
	{
		ASSERT(!m_audioSources.contains(audioSource));
		m_audioSources.insert(audioSource);
	}

	void AudioManager::DeregisterAudioSource(AudioSourceBase* audioSource)
	{
		ASSERT(m_audioSources.contains(audioSource));
		m_audioSources.erase(audioSource);
	}

	void AudioManager::InitSoundsFromFolder(const char* dir)
	{
		// Free up any previously loaded sounds
		ReleaseSounds();

		// Load manifest of files in directory
		Manifest manifest;
		AssetManifest::GetManifest(dir, ".mp3", manifest);

		// Load each discovered file into expected slot
		for (int i = 0; i < manifest.size(); i++)
		{
			if (manifest[i] != "")
			{
				LoadSound(manifest[i].c_str(), i);
			}
		}
	}

	bool AudioManager::LoadSound(const char* filepath, int slot)
	{
		ASSERT(!m_audioBufferSlots.contains(slot));

		// Load checked data from supplied path
		SF_INFO sfinfo;
		SNDFILE* sndfile = sf_open(filepath, SFM_READ, &sfinfo);
		if (!sndfile)
		{
			LOG(std::string("Failed to open audio file: ") + filepath + ". Error: " + sf_strerror(sndfile));
			return false;
		}
		if (sfinfo.frames < 1 || sfinfo.frames >(sf_count_t)(INT_MAX / sizeof(short)) / sfinfo.channels)
		{
			LOG(std::string("Bad sample count in file: ") + filepath);
			sf_close(sndfile);
			return false;
		}

		// Determine OpenAL audio format
		ALenum format = AudioUtils::GetFormatFromSndFile(sndfile, sfinfo);
		if (!format)
		{
			LOG(std::string("Unsupported channel count (") + std::to_string(sfinfo.channels) + ") in file: " + filepath);
			sf_close(sndfile);
			return false;
		}

		// Decode full file into local buffer
		short* membuf = new short[sfinfo.frames * sfinfo.channels];
		sf_count_t numLoadedFrames = sf_readf_short(sndfile, membuf, sfinfo.frames);
		if (numLoadedFrames != sfinfo.frames)
		{
			delete[] membuf;
			sf_close(sndfile);
			LOG(std::string("Failed to load audio from file: ") + filepath);
			return false;
		}

		// Assign local buffer data to OpenAL buffer
		ALuint buffer = 0;
		alGenBuffers(1, &buffer);
		ALsizei totalBytes = (numLoadedFrames * sfinfo.channels) * sizeof(short);
		alBufferData(buffer, format, membuf, totalBytes, sfinfo.samplerate);

		// Clean up local buffer
		delete[] membuf;
		sf_close(sndfile);

		// Handle any errors
		ALenum err = alGetError();
		if (err != AL_NO_ERROR)
		{
			LOG(std::string("OpenAL Error: ") + alGetString(err));
			if (buffer && alIsBuffer(buffer))
			{
				alDeleteBuffers(1, &buffer);
			}
			return false;
		}

		// Store handle to buffer
		m_audioBufferSlots.insert({ slot, buffer });

		// Return success
		return true;
	}

	void AudioManager::GlobalPlaySound(int slot)
	{
		m_soundSource->SetSound(slot);
		m_soundSource->Play();
	}

	void AudioManager::GlobalStopSound()
	{
		m_soundSource->Stop();
	}

	void AudioManager::GlobalPlayStream(const char* filepath)
	{
		m_streamSource->SetStreamingFile(filepath);
		m_streamSource->Play();
	}

	void AudioManager::GlobalStopStream()
	{
		m_streamSource->Stop();
	}

	void AudioManager::StopAllAudio()
	{
		for (AudioSourceBase* source : m_audioSources)
		{
			source->Stop();
		}
		m_playingStreams.clear();
	}

	u32 AudioManager::GetBufferForSoundId(int soundId) const
	{
		ASSERT(m_audioBufferSlots.contains(soundId));
		return m_audioBufferSlots.at(soundId);
	}

	void AudioManager::AddToPlayingStreams(StreamSource& stream)
	{
		m_playingStreams.push_back(&stream);
	}

	void AudioManager::UpdatePlayingStreams()
	{
		std::vector<StreamSource*> finishedStreams;
		for (int i = m_playingStreams.size() - 1; i >= 0; i--)
		{
			StreamSource* stream = m_playingStreams[i];
			if (!stream->UpdateAudioStream())
			{
				std::iter_swap(m_playingStreams.begin() + i, m_playingStreams.end() - 1);
				m_playingStreams.pop_back();
			}
		}
	}
	
	void AudioManager::ReleaseSounds()
	{
		for (const auto& bufferSlot : m_audioBufferSlots)
		{
			ASSERT(bufferSlot.second);
			alDeleteBuffers(1, &bufferSlot.second);
		}
		m_audioBufferSlots.clear();
	}
}