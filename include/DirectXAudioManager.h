#pragma once
#include "AudioManager.h"

#include <Audio.h>
#include <queue>

namespace BlackMagic
{
	class DirectXAudioManager :
		public AudioManager
	{
	private:
		virtual void PlayOneShotInternal(AudioFile file, float relativeVolume) override;
		virtual void PlayBGMInternal(AudioFile file, float relativeVolume) override;
		virtual void PauseBGMInternal() override;
		virtual void StopBGMInternal() override;
		virtual void ResumeBGMInternal(float relativeVolume) override;
	public:
		std::unique_ptr<DirectX::AudioEngine> audioEngine;
		std::unique_ptr<DirectX::SoundEffectInstance> BGM;

		DirectXAudioManager();
		~DirectXAudioManager();
		virtual void UpdateAudio() override;
	};

}