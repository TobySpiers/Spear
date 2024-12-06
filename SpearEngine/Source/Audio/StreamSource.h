#pragma once
#include "Core/Core.h"
#include "al.h"
#include "sndfile.hh"
#include "AudioSourceBase.h"

namespace Spear
{
	// Class for playing long sounds. Audio is continually streamed in and out of memory.
	class StreamSource : public AudioSourceBase
	{
		friend class AudioManager;
		static constexpr int NUM_BUFFERS = 4;
		static constexpr int STREAMBUFFER_SIZE = 4096; // Size of reserved internal memory used to stream data into OpenAL buffers. Consider increasing if issues arise.
	
	public:
		SERIALIZABLE_DERIVED(StreamSource, AudioSourceBase, m_streamingFilepath, m_playing);
		StreamSource();
		virtual ~StreamSource();
		virtual void PostDeserialize() override;

		bool SetStreamingFile(const char* filepath);
		bool OnFinished();

		virtual void Play() override;
		virtual void Stop() override;

	private:
		// Internal usage only
		void ReleaseStreamingFile();
		std::string m_streamingFilepath;

		// Returns true while playback is active
		bool UpdateAudioStream();
		bool m_playing{ false };

		// OpenAL
		u32 m_buffers[NUM_BUFFERS];
		int m_state{ 0 };

		// Streaming data
		short* m_streambuffer{ nullptr };
		SNDFILE* m_sndfile{ nullptr };
		SF_INFO m_sndfileInfo;
		ALenum m_format{AL_NONE};
	};
}