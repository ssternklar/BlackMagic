#include "AudioManager.h"

using namespace BlackMagic;

AudioFX* BlackMagic::AudioManager::PlaySound(AudioFile file, float loudnessScale)
{
	AudioFX fx(file, loudnessScale * upperLoudness);

	sounds[currentSounds] = fx;
	currentSounds++;
	PlaySoundInternal(fx);
	return &sounds[currentSounds - 1];
}

void BlackMagic::AudioManager::StopSound(AudioFX * fileToStop)
{
	int index = static_cast<int>(fileToStop - sounds);
	if (index < 0 || index >= MAX_SOUNDS)
	{
		return;
	}
	StopSoundInternal(sounds[index]);
	currentSounds--;
	sounds[index] = sounds[currentSounds];
	sounds[currentSounds] = AudioFX(0, 0);
}

void BlackMagic::AudioManager::StopAllSounds()
{
}

void BlackMagic::AudioManager::PlayBGM(AudioFile file, float loudnessScale)
{
}

void BlackMagic::AudioManager::StopBGM()
{
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
