#include "RacingStartJob.h"
#include "ThreadManager.h"
#include "AudioManager.h"
#include "PlatformBase.h"
using namespace BlackMagic;
RacingStartJob::RacingStartJob(BlackMagic::AssetPointer<BlackMagic::Material>* lightMats, BlackMagic::Entity** lights, BlackMagic::AssetPointer<BlackMagic::WAVFile>* sounds, bool* toChangeOnceDone)
{
	this->lightMats = lightMats;
	this->lights = lights;
	this->sounds = sounds;
	this->toChangeOnceDone = toChangeOnceDone;
	cleanupSelf = true;
}

void RacingStartJob::Run()
{
	int nextLight = 0;
	PlatformBase* platform = PlatformBase::GetSingleton();
	for (int i = 0; i < 3; i++)
	{
		lights[i]->_material = *lightMats[0];
	}

	platform->GetThreadManager()->SleepThisThread(2000);
	for (int i = 0; i < 3; i++)
	{
		lights[i]->_material = *lightMats[1];
		platform->GetAudioManager()->PlayOneShot(sounds[0].get(), 1);
		platform->GetThreadManager()->SleepThisThread(1000);
	}
	lights[0]->_material = *lightMats[2];
	platform->GetAudioManager()->PlayOneShot(sounds[1].get(), 1);
	*toChangeOnceDone = true;
}

RacingStartJob::~RacingStartJob()
{
}
