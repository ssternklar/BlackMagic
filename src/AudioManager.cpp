#include "AudioManager.h"

using namespace BlackMagic;

byte BlackMagic::AudioManager::PlaySound(AudioFile file, float loudnessScale)
{
	PlaySoundInternal(&sounds[currentSounds]);
	return sounds[currentSounds].id;
}

void BlackMagic::AudioManager::StopSound(int index)
{
	if (index >= currentSounds) return;
	StopSoundInternal(&sounds[index]);
	currentSounds--;
	sounds[index] = sounds[currentSounds];
	sounds[currentSounds] = AudioFX(0, 0, 0);
}

byte BlackMagic::AudioManager::RequestPlay(AudioFile file, float loudnessScale)
{
	AudioFX fx(file, currentIndex++, loudnessScale * upperLoudness);
	sounds[currentSounds] = fx;
	currentSounds++;
}

void BlackMagic::AudioManager::RequestStop(byte identifierToStop)
{
	int index = 0;
	for (; index < currentSounds && sounds[index].id != identifierToStop; index++);
	sounds[index].state = AudioFX::State::Stopping;
}

void BlackMagic::AudioManager::StopAllSounds()
{
	for (int i = 0; i < currentSounds; i++)
	{
		StopSoundInternal(&sounds[i]);
		sounds[i] = AudioFX(0, 0, 0);
	}
}

void BlackMagic::AudioManager::PlayBGM(AudioFile file, float loudnessScale)
{
	if (BGM.id != 0)
	{
		StopBGMInternal();
	}
	BGM = AudioFX(file, 1, loudnessScale * upperLoudness);
}

void BlackMagic::AudioManager::StopBGM()
{
	BGM.state = AudioFX::State::Stopping;
}

void BlackMagic::AudioManager::UpdateRunningSounds()
{
	for (int i = 0; i < currentSounds; i++)
	{
		if (sounds[i].state == AudioFX::State::ToPlay)
		{
			PlaySoundInternal(&sounds[i]);
		}
		else if (sounds[i].state == AudioFX::State::Stopping)
		{
			StopSound(i);
		}
	}
	if (BGM.state == AudioFX::State::ToPlay)
	{
		PlayBGMInternal(&BGM);
		BGM.state = AudioFX::State::Playing;
	}
	else if (BGM.state == AudioFX::State::Stopping)
	{
		StopBGMInternal();
		BGM.state = AudioFX::State::Stopped;
	}
}

AudioManager::AudioManager(PlatformBase* base) : platformBase(base)
{
}


AudioManager::~AudioManager()
{
}
