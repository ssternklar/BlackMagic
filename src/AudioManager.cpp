#include "AudioManager.h"

using namespace BlackMagic;

byte BlackMagic::AudioManager::PlaySound(AudioFile file, float loudnessScale)
{
	AudioFX fx(file, currentIndex++, loudnessScale * upperLoudness);
	sounds[currentSounds] = fx;
	currentSounds++;
	PlaySoundInternal(fx);
	return fx.id;
}

void BlackMagic::AudioManager::StopSound(byte fileToStop)
{

	int index = 0;
	for (; index < currentSounds && sounds[index].id != fileToStop; index++);
	if (index >= currentSounds) return;
	StopSoundInternal(sounds[index]);
	currentSounds--;
	sounds[index] = sounds[currentSounds];
	sounds[currentSounds] = AudioFX(0, 0, 0);
}

void BlackMagic::AudioManager::StopAllSounds()
{
	for (int i = 0; i < currentSounds; i++)
	{
		StopSoundInternal(sounds[i]);
		sounds[i] = AudioFX(0, 0, 0);
	}
}

void BlackMagic::AudioManager::PlayBGM(AudioFile file, float loudnessScale)
{
	if (BGM.id != 0)
	{
		StopBGM();
	}
	BGM = AudioFX(file, 1, loudnessScale * upperLoudness);
	PlayBGMInternal(BGM);
}

void BlackMagic::AudioManager::StopBGM()
{
	StopBGMInternal();
	BGM = AudioFX(0, 0, 0);
}

void BlackMagic::AudioManager::UpdateRunningSounds()
{
	for (int i = 0; i < currentSounds; i++)
	{
		if (sounds[i].state == AudioFX::State::ToPlay)
		{
			PlaySoundInternal(sounds[i]);
		}
		else if (sounds[i].state == AudioFX::State::Stopping)
		{
			StopSoundInternal(sounds[i]);
		}
	}
	if (BGM.state == AudioFX::State::ToPlay)
	{
		PlayBGMInternal(BGM);
	}
	else if (BGM.state == AudioFX::State::Stopping)
	{
		StopBGMInternal();
	}
}

AudioManager::AudioManager()
{
}


AudioManager::~AudioManager()
{
}
