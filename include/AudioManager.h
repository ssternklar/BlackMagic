#pragma once

#include "allocators\globals.h"
#include "Handles.h"

namespace BlackMagic
{
	class PlatformBase;
	class AudioManager
	{
	public:
		virtual void PlayOneShot(AudioFile file, float relativeVolume) = 0;
		virtual void PlayBGM(AudioFile file, float relativeVolume) = 0;
		virtual void PauseBGM() = 0;
		virtual void StopBGM() = 0;
		virtual void ResumeBGM(float relativeVolume) = 0;
		virtual void UpdateAudio() = 0;
		virtual ~AudioManager(){}
	};
}