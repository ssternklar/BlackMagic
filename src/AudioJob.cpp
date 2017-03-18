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
			manager->PlayBGM(fileToPlay, relativeVolume);
			break;
		case 1:
			manager->PauseBGM();
			break;
		case 2:
			manager->StopBGM();
			break;
		case 3:
			manager->ResumeBGM(relativeVolume);
			break;
		default:
			break;
		}
	}
	else
	{
		manager->PlayOneShot(fileToPlay, relativeVolume);
	}
}
