#include "AudioManager.h"
#include "ServiceLocator.h"
#include "AssetManifest.h"
#include "AudioUtils.h"
#include "al.h"
#include "alc.h"
#include "alext.h"
#include "sndfile.hh"

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

		ASSERT(!m_pGlobalSoundSource);
		m_pGlobalSoundSource = new SoundSource;
		ASSERT(!m_pGlobalStreamSource);
		m_pGlobalStreamSource = new StreamSource;
	}

	AudioManager::~AudioManager()
	{
		delete m_pGlobalSoundSource;
		m_pGlobalSoundSource = nullptr;
		delete m_pGlobalStreamSource;
		m_pGlobalStreamSource = nullptr;

		ReleaseSounds();

		alcMakeContextCurrent(nullptr);
		alcDestroyContext(m_context);
		alcCloseDevice(m_device);
	}

	AudioManager& AudioManager::Get()
	{
		return ServiceLocator::GetAudioManager();
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
		m_pGlobalSoundSource->PlaySound(slot);
	}

	void AudioManager::GlobalStopSound()
	{
		m_pGlobalSoundSource->StopSound();
	}

	void AudioManager::GlobalPlayStream(const char* filepath)
	{
		m_pGlobalStreamSource->SetStreamingFile(filepath);
		m_pGlobalStreamSource->PlayStream();
	}

	void AudioManager::GlobalStopStream()
	{
		m_pGlobalStreamSource->StopStream();
	}

	void AudioManager::StopAllAudio()
	{
		m_pGlobalSoundSource->StopSound();
		for (StreamSource* stream : m_activeStreams)
		{
			stream->StopStream();
		}
		m_activeStreams.clear();
	}

	u32 AudioManager::GetBufferForId(int soundId) const
	{
		ASSERT(m_audioBufferSlots.contains(soundId));
		return m_audioBufferSlots.at(soundId);
	}

	void AudioManager::RegisterActiveStream(StreamSource& stream)
	{
		m_activeStreams.insert(&stream);
	}

	void AudioManager::UpdateStreamingSounds()
	{
		std::vector<StreamSource*> finishedStreams;
		for (StreamSource* stream : m_activeStreams)
		{
			if (!stream->UpdateAudioStream())
			{
				finishedStreams.push_back(stream);
			}
		}

		while (finishedStreams.size())
		{
			m_activeStreams.erase(finishedStreams.back());
			finishedStreams.pop_back();
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