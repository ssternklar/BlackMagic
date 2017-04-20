#include "AudioJob.h"
#include "PlatformBase.h"

using namespace BlackMagic;

void BlackMagic::AudioJob::Run()
{
	AudioManager* manager = PlatformBase::GetSingleton()->GetAudioManager();
	if (isBGM)
	{
		switch (bgmPlayPauseStopResume)
		{
		case 0:
			manager->PlayBGMInternal(fileToPlay, channelCount, relativeVolume);
			break;
		case 1:
			manager->PauseBGMInternal();
			break;
		case 2:
			manager->StopBGMInternal();
			break;
		case 3:
			manager->ResumeBGMInternal(relativeVolume);
			break;
		default:
			break;
		}
	}
	else
	{
		manager->PlayOneShotInternal(fileToPlay, channelCount, relativeVolume);
	}
}
