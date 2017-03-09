#pragma once

#include "allocators\globals.h"
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
		byte id;
		AudioFX(AudioFile file, byte identifier, float loudness) : file(file), id(identifier), actualLoudness(loudness), state(ToPlay) {}
		AudioFX() : AudioFX(0, 0, 0) {}
	};

	class PlatformBase;
	class AudioManager
	{
	protected:
		static const unsigned int MAX_SOUNDS = 4;
		unsigned int currentSounds = 0;
		AudioFX sounds[MAX_SOUNDS];
		AudioFX BGM;
		byte currentIndex = 0;
		byte PlaySound(AudioFile file, float loudnessScale);
		void StopSound(int index);
		virtual void PlaySoundInternal(AudioFX* effect) = 0;
		virtual void PlayBGMInternal(AudioFX* bgm) = 0;
		virtual void StopSoundInternal(AudioFX* file) = 0;
		virtual void StopBGMInternal() = 0;
	public:
		PlatformBase* platformBase;
		float upperLoudness = 40;

		byte RequestPlay(AudioFile file, float loudnessScale);
		void RequestStop(byte identifierToStop);
		void StopAllSounds();
		void PlayBGM(AudioFile file, float loudnessScale);
		void StopBGM();

		void UpdateRunningSounds();

		AudioManager(PlatformBase* base);
		~AudioManager();
	};

}