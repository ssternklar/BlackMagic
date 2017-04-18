#pragma once

#include "allocators\globals.h"
#include "Handles.h"

namespace BlackMagic
{
	class AudioManager
	{
		friend class AudioJob;
	protected:
		virtual void PlayOneShotInternal(AudioFile file, float relativeVolume) = 0;
		virtual void PlayBGMInternal(AudioFile file, float relativeVolume) = 0;
		virtual void PauseBGMInternal() = 0;
		virtual void StopBGMInternal() = 0;
		virtual void ResumeBGMInternal(float relativeVolume) = 0;
	public:
		void PlayOneShot(AudioFile file, float relativeVolume);
		void PlayBGM(AudioFile file, float relativeVolume);
		void PauseBGM();
		void StopBGM();
		void ResumeBGM(float relativeVolume);
		virtual void UpdateAudio() = 0;
		virtual ~AudioManager(){}
	};
}