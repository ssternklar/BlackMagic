#include "AudioManager.h"
#include "PlatformBase.h"
using namespace BlackMagic;

void BlackMagic::AudioManager::PlayOneShot(AudioFile file, int channelCount, float relativeVolume)
{
	PlatformBase::GetSingleton()->GetThreadManager()->CreateAudioJob(false, file, channelCount, relativeVolume, 0);
}

void BlackMagic::AudioManager::PlayBGM(AudioFile file, int channelCount, float relativeVolume)
{
	PlatformBase::GetSingleton()->GetThreadManager()->CreateAudioJob(true, file, channelCount, relativeVolume, 0);
}

void BlackMagic::AudioManager::PauseBGM()
{
	PlatformBase::GetSingleton()->GetThreadManager()->CreateAudioJob(true, 0, 0, 0, 1);
}

void BlackMagic::AudioManager::StopBGM()
{
	PlatformBase::GetSingleton()->GetThreadManager()->CreateAudioJob(true, 0, 0, 0, 2);
}

void BlackMagic::AudioManager::ResumeBGM(float relativeVolume)
{
	PlatformBase::GetSingleton()->GetThreadManager()->CreateAudioJob(true, 0, 0, relativeVolume, 3);
}
