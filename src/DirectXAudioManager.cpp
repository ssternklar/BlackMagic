#include "DirectXAudioManager.h"
#include <iostream>

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

void BlackMagic::DirectXAudioManager::PlayOneShotInternal(WAVFile* file, float relativeVolume)
{
	if (map.find(file) == map.end())
	{
		byte* x = (byte*)::operator new(sizeof(WAVFile) + file->dataSize);
		WAVFile* tmpWav = (WAVFile*)x;
		*tmpWav = *file;
		tmpWav->pcmData = x + sizeof(WAVFile);
		memcpy_s(tmpWav->pcmData, file->dataSize, file->pcmData, file->dataSize);

		auto bs = std::unique_ptr<uint8_t[]>((uint8_t*)x);

		std::shared_ptr<SoundEffect> effect = std::make_shared<SoundEffect>(
			audioEngine.get(),
			bs,
			reinterpret_cast<WAVEFORMATEX*>(file),
			(uint8_t*)file->pcmData,
			file->dataSize);
		map[file] = effect;
	}
	map[file]->Play(relativeVolume, 0, 0);
}

void BlackMagic::DirectXAudioManager::PlayBGMInternal(WAVFile* file, float relativeVolume)
{
	PauseBGMInternal();
	if (map.find(file) == map.end())
	{
		byte* x = (byte*)::operator new(sizeof(WAVFile) + file->dataSize);
		WAVFile* tmpWav = (WAVFile*)x;
		*tmpWav = *file;
		tmpWav->pcmData = x + sizeof(WAVFile);
		memcpy_s(tmpWav->pcmData, file->dataSize, file->pcmData, file->dataSize);

		auto bs = std::unique_ptr<uint8_t[]>((uint8_t*)x);

		std::shared_ptr<SoundEffect> effect = std::make_shared<SoundEffect>(
			audioEngine.get(),
			bs,
			reinterpret_cast<WAVEFORMATEX*>(file),
			(uint8_t*)file->pcmData,
			file->dataSize);
		map[file] = effect;
	}
	BGM = map[file]->CreateInstance();
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
