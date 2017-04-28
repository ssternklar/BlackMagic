#pragma once
#include "AudioManager.h"
#include "WAVFile.h"
#include <Audio.h>
#include <unordered_map>

namespace BlackMagic
{
	class DirectXAudioManager :
		public AudioManager
	{
	protected:
		virtual void PlayOneShotInternal(WAVFile* file, float relativeVolume) override;
		virtual void PlayBGMInternal(WAVFile* file, float relativeVolume) override;
		virtual void PauseBGMInternal() override;
		virtual void StopBGMInternal() override;
		virtual void ResumeBGMInternal(float relativeVolume) override;
	public:
		std::unique_ptr<DirectX::AudioEngine> audioEngine;
		std::unique_ptr<DirectX::SoundEffectInstance> BGM;
		std::unordered_map<WAVFile*, std::shared_ptr<DirectX::SoundEffect>> map;

		DirectXAudioManager();
		~DirectXAudioManager();
		virtual void UpdateAudio() override;
	};

}