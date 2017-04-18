#include "DirectXAudioManager.h"

using namespace BlackMagic;
using namespace DirectX;

DirectXAudioManager::DirectXAudioManager()
{
	AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
	audioEngine = std::make_unique<AudioEngine>(eflags);
}

DirectXAudioManager::~DirectXAudioManager()
{
	audioEngine.release();
}

void BlackMagic::DirectXAudioManager::PlayOneShotInternal(AudioFile file, float relativeVolume)
{
	file.GetAs<SoundEffect*>()->Play(relativeVolume, 0, 0);
}

void BlackMagic::DirectXAudioManager::PlayBGMInternal(AudioFile file, float relativeVolume)
{
	PauseBGM();
	BGM = file.GetAs<SoundEffect*>()->CreateInstance();
	BGM->SetVolume(relativeVolume);
	BGM->Play(true);
}

void BlackMagic::DirectXAudioManager::PauseBGMInternal()
{
	if (BGM)
	{
		BGM->Pause();
	}
}

void BlackMagic::DirectXAudioManager::StopBGMInternal()
{
	if (BGM)
	{
		BGM->Stop();
	}
}

void BlackMagic::DirectXAudioManager::ResumeBGMInternal(float relativeVolume)
{
	if(BGM)
	{
		if(BGM->GetState() == SoundState::PAUSED)
		{
			BGM->Resume();
		}
		else if (BGM->GetState() == SoundState::STOPPED)
		{
			BGM->Play(true);
		}
		BGM->SetVolume(relativeVolume);
	}
}

void BlackMagic::DirectXAudioManager::UpdateAudio()
{
	audioEngine->Update();
}
