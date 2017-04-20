#pragma once

#include "allocators\globals.h"
#include "Handles.h"
#include <assert.h>

namespace BlackMagic {

class WAVFile
{
public:
	AudioFile audioFile;
	void* pcmData;
	int dataSize;
	int channelCount;
	int samplesPerSecond;
	int bitsPerSample;

	WAVFile(AudioFile audioFile);

};

};