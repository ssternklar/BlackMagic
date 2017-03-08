#pragma once

#include "Handles.h"

namespace BlackMagic
{
	class AudioFX
	{
	public:
		enum State
		{
			ToPlay,
			Playing,
			Stopping,
			Stopped
		};
		AudioFile file;
		AudioChannel channel;
		State state = Stopped;
		float actualLoudness;
		float time;
		AudioFX(AudioFile file, float loudness) : file(file), actualLoudness(loudness), state(ToPlay), time(0) {};
	};

	class PlatformBase;
	class AudioManager
	{
	protected:
		static const unsigned int MAX_SOUNDS = 4;
		unsigned int currentSounds = 0;
		AudioFX sounds[MAX_SOUNDS];
		AudioFX BGM;
		virtual void PlaySoundInternal(AudioFX effect) = 0;
		virtual void PlayBGMInternal(AudioFX bgm) = 0;
		virtual void StopSoundInternal(AudioFX file) = 0;
		virtual void StopBGMInternal() = 0;
	public:
		PlatformBase* platformBase;
		float upperLoudness = 40;

		AudioFX* PlaySound(AudioFile file, float loudnessScale);
		void StopSound(AudioFX* fileToStop);
		void StopAllSounds();
		void PlayBGM(AudioFile file, float loudnessScale);
		void StopBGM();

		void UpdateRunningSounds();

		AudioManager();
		~AudioManager();
	};

}