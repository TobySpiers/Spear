#pragma once
#include "Core.h"
#include "al.h"
#include "sndfile.hh"

namespace Spear
{
	// Class for playing long sounds. Audio is continually streamed in and out of memory.
	class StreamSource
	{
		friend class AudioManager;
		static constexpr int NUM_BUFFERS = 4;
		static constexpr int STREAMBUFFER_SIZE = 4096; // Size of reserved internal memory used to stream data into OpenAL buffers. Consider increasing if issues arise.
	
	public:
		StreamSource();
		~StreamSource();

		bool SetStreamingFile(const char* filepath);

		void PlayStream(bool bLooping = true);
		void StopStream();

		void OnFinished();

	private:
		// Internal usage only
		void ReleaseStreamingFile();

		// Returns true while playback is active
		bool UpdateAudioStream();
		bool bPlaybackActive{ false };
		bool bLooping{ false };

		// OpenAL
		u32 m_source{ 0 };
		u32 m_buffers[NUM_BUFFERS];
		int m_state{ 0 };

		// Streaming data
		short* m_streambuffer{ nullptr };
		SNDFILE* m_sndfile{ nullptr };
		SF_INFO m_sndfileInfo;
		ALenum m_format{AL_NONE};

		// Source data
		float m_pitch{ 1.f };
		float m_gain{ 1.f };
		float m_position[3] = { 0, 0, 0 };
		float m_velocity[3] = { 0, 0, 0 };
		bool m_looping{ false };
	};
}