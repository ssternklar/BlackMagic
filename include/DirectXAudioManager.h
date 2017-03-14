#pragma once
#include "AudioManager.h"

#include <Audio.h>
#include <queue>

namespace BlackMagic
{
	class DirectXAudioManager :
		public AudioManager
	{
	public:
		std::queue<AudioFile> queuedOneShots;
		std::unique_ptr<DirectX::AudioEngine> audioEngine;
		std::unique_ptr<DirectX::SoundEffectInstance> BGM;

		DirectXAudioManager();
		~DirectXAudioManager();
		virtual void PlayOneShot(AudioFile file, float relativeVolume) override;
		virtual void PlayBGM(AudioFile file, float relativeVolume) override;
		virtual void PauseBGM() override;
		virtual void StopBGM() override;
		virtual void ResumeBGM(float relativeVolume) override;
		virtual void UpdateAudio() override;
	};

}