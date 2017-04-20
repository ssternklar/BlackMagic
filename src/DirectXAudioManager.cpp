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

void BlackMagic::DirectXAudioManager::PlayOneShotInternal(AudioFile file, int channelCount, float relativeVolume)
{
	if (map.find(file.GetAs<void*>()) == map.end())
	{
		WAVFile wavFile(file);
		WAVEFORMATEX ex;
		ex.wFormatTag = WAVE_FORMAT_PCM;
		ex.nChannels = wavFile.channelCount;
		ex.nSamplesPerSec = wavFile.samplesPerSecond;
		ex.nBlockAlign = (wavFile.channelCount * wavFile.bitsPerSample) / 8;
		ex.nAvgBytesPerSec = ex.nBlockAlign * wavFile.samplesPerSecond;
		ex.wBitsPerSample = wavFile.bitsPerSample;
		ex.cbSize = 0;
		std::shared_ptr<SoundEffect> effect = std::make_shared<SoundEffect>(audioEngine.get(), std::unique_ptr<uint8_t[]>(nullptr), &ex, (uint8_t*)wavFile.pcmData, wavFile.dataSize);
		map[file.GetAs<void*>()] = effect;
	}
	map[file.GetAs<void*>()]->Play(relativeVolume, 0, 0);
}

void BlackMagic::DirectXAudioManager::PlayBGMInternal(AudioFile file, int channelCount, float relativeVolume)
{
	PauseBGM();
	if (map.find(file.GetAs<void*>()) == map.end())
	{
		WAVFile wavFile(file);
		WAVEFORMATEX ex;
		ex.wFormatTag = WAVE_FORMAT_PCM;
		ex.nChannels = wavFile.channelCount;
		ex.nSamplesPerSec = wavFile.samplesPerSecond;
		ex.nBlockAlign = (wavFile.channelCount * wavFile.bitsPerSample) / 8;
		ex.nAvgBytesPerSec = ex.nBlockAlign * wavFile.samplesPerSecond;
		ex.wBitsPerSample = wavFile.bitsPerSample;
		ex.cbSize = 0;
		std::shared_ptr<SoundEffect> effect = std::make_shared<SoundEffect>(audioEngine.get(), std::unique_ptr<uint8_t[]>(nullptr), &ex, (uint8_t*)wavFile.pcmData, wavFile.dataSize);
		map[file.GetAs<void*>()] = effect;
	}
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
