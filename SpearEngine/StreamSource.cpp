#include "StreamSource.h"
#include "AudioManager.h"
#include "AudioUtils.h"

Spear::StreamSource::StreamSource()
{
	// Configure source
	alGenSources(1, &m_source);
	alSourcef(m_source, AL_PITCH, m_pitch);
	alSourcef(m_source, AL_GAIN, m_gain);
	alSource3f(m_source, AL_POSITION, m_position[0], m_position[1], m_position[2]);
	alSource3f(m_source, AL_VELOCITY, m_velocity[0], m_velocity[1], m_velocity[2]);
	alSourcei(m_source, AL_LOOPING, m_looping);
	alSourcei(m_source, AL_BUFFER, 0);

	// Init multiple buffers for streaming/juggling data
	alGenBuffers(NUM_BUFFERS, m_buffers);

	m_streambuffer = new short[STREAMBUFFER_SIZE];
}

Spear::StreamSource::~StreamSource()
{
	delete[] m_streambuffer;
	m_streambuffer = nullptr;

	ReleaseStreamingFile();

	alDeleteBuffers(NUM_BUFFERS, m_buffers);
	alDeleteSources(1, &m_source);
}

bool Spear::StreamSource::SetStreamingFile(const char* filepath)
{
	// Clear any existing file
	ReleaseStreamingFile();

	// Open new file and assert success
	m_sndfile = sf_open(filepath, SFM_READ, &m_sndfileInfo);
	ASSERT(m_sndfile);

	// Determine OpenAL audio format
	m_format = AudioUtils::GetFormatFromSndFile(m_sndfile, m_sndfileInfo);
	if (!m_format)
	{
		LOG(std::string("Unsupported channel count (") + std::to_string(m_sndfileInfo.channels) + ") in file: " + filepath);
		sf_close(m_sndfile);
		return false;
	}
}

void Spear::StreamSource::ReleaseStreamingFile()
{
	if (m_sndfile)
	{
		sf_close(m_sndfile);
		m_sndfile = nullptr;
	}
}

void Spear::StreamSource::PlayStream(bool in_bLooping)
{
	// ensure SetStreamingFile has been succesfully called 
	ASSERT(m_sndfile);

	// Position sndfile cursor at start of audio file
	sf_seek(m_sndfile, 0, SEEK_SET);

	// Reset playback
	bPlaybackActive = true;
	bLooping = in_bLooping;
	alSourceRewind(m_source);
	alSourcei(m_source, AL_BUFFER, 0); // reset active buffer

	// Load up initial buffers from file start
	for (int i = 0; i < NUM_BUFFERS; i++)
	{
		// Make sure some data remains to queue up
		sf_count_t numLoadedFrames = sf_readf_short(m_sndfile, m_streambuffer, STREAMBUFFER_SIZE / m_sndfileInfo.channels);
		if (numLoadedFrames < 1)
		{
			break;
		}

		// Load into OpenAL buffer
		ALsizei totalBytes = (numLoadedFrames * m_sndfileInfo.channels) * sizeof(short);
		alBufferData(m_buffers[i], m_format, m_streambuffer, totalBytes, m_sndfileInfo.samplerate);
	}
	ASSERT(!alGetError());

	// Queue buffers and begin playback
	alSourceQueueBuffers(m_source, NUM_BUFFERS, m_buffers);
	alSourcePlay(m_source);
	ASSERT(!alGetError());

	// Register active stream with AudioManager for automatic updates/controls
	AudioManager::Get().RegisterActiveStream(*this);
}

bool Spear::StreamSource::UpdateAudioStream()
{
	if (!bPlaybackActive)
	{
		return false;
	}

	// Get number of processed (finished) buffers
	int buffersProcessed{ 0 };
	alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &buffersProcessed);

	// For each fully processed buffer (if any exist), update data & requeue...
	while (buffersProcessed--)
	{
		// Unqueue a processed buffer and grab its ID
		ALuint processedBufferID;
		alSourceUnqueueBuffers(m_source, 1, &processedBufferID);

		// Read next audio chunk from file
		sf_count_t numLoadedFrames = sf_readf_short(m_sndfile, m_streambuffer, STREAMBUFFER_SIZE / m_sndfileInfo.channels);
		if (numLoadedFrames < 1)
		{
			break;
		}

		// Upload data into OpenAL buffer and requeue
		ALsizei totalBytes = (numLoadedFrames * m_sndfileInfo.channels) * sizeof(short);
		alBufferData(processedBufferID, m_format, m_streambuffer, totalBytes, m_sndfileInfo.samplerate);
		alSourceQueueBuffers(m_source, 1, &processedBufferID);
	}

	// Check state
	alGetSourcei(m_source, AL_SOURCE_STATE, &m_state);
	if (m_state == AL_STOPPED)
	{
		ALint remainingQueue;
		alGetSourcei(m_source, AL_BUFFERS_QUEUED, &remainingQueue);
		if (!remainingQueue)
		{
			// If no queue remaining, sound is finished
			if (bLooping)
			{
				PlayStream(bLooping);
			}
			else
			{
				OnFinished();
				return false;
			}
		}
		else
		{
			// If queue remains, must have previously ran out of buffer. Resume playback
			alSourcePlay(m_source);
			ASSERT(!alGetError());
		}
	}
}

// Sound was manually stopped
void Spear::StreamSource::StopStream()
{
	alSourceStop(m_source);
	bPlaybackActive = false;
}

// Sound reached end organically
void Spear::StreamSource::OnFinished()
{
	bPlaybackActive = false;
}
