#pragma once

#include "allocators\globals.h"
#include "Handles.h"
#include <assert.h>
#include <stdint.h>
namespace BlackMagic {

class WAVFile
{
public:
	uint16_t wavFormat = 1; // WAVEFORMATEX compatibility, wFormatTag
	uint16_t channelCount;
	uint32_t samplesPerSecond;
	uint32_t avgBytesPerSecond; //WAVEFORMATEX compatibility, nAvgBytesPerSec
	uint16_t blockAlign; // WAVEFORMATEX compatibility, nBlockAlign
	uint16_t bitsPerSample;
	uint16_t cbSize = 0; // WAVEFORMATEX compatibility, cbSize
	uint32_t dataSize;
	void* pcmData;

	WAVFile(byte* audioFile);

};

};