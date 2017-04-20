#pragma once
#include "GenericJob.h"
#include "AudioManager.h"
namespace BlackMagic
{
	class AudioJob : public GenericJob
	{
	public:
		AudioFile fileToPlay;
		float relativeVolume;
		bool isBGM;
		int channelCount;
		byte bgmPlayPauseStopResume; // 0 is play, 1 is pause, 2 is stop, 3 is resume
		virtual void Run() override;
	};
}