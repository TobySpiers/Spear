#pragma once
#include "Core/Core.h"
#include "Core/Serializer.h"

namespace Spear
{
	enum class AudioCategory : int
	{
		Audio_Unknown,
		Audio_SFX,
		Audio_Music
	};

	class AudioSourceBase
	{
	public:
		SERIALIZABLE_BASE(AudioSourceBase,
			m_position,
			m_velocity,
			m_pitch,
			m_gain,
			m_looping);

		AudioSourceBase();
		virtual ~AudioSourceBase();

		virtual void SetPosition(const Vector3f& position);
		virtual void SetVelocity(const Vector3f& velocity);
		virtual void SetPitch(float pitch);
		virtual void SetGain(float gain);
		virtual void SetLooping(bool looping);

		virtual void Play() = 0;
		virtual void Stop() = 0;

		Vector3f m_position{ 0.f, 0.f, 0.f };
		Vector3f m_velocity{ 0.f, 0.f, 0.f };
		float m_pitch{ 1.f };
		float m_gain{ 1.f };
		bool m_looping{ false };
		AudioCategory m_category = AudioCategory::Audio_Unknown;
		u32 m_source{ 0 };

	protected:
		virtual void RefreshSourceSettings();
	};
}