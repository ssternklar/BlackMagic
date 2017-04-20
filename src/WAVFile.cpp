#include "WAVFile.h"

using namespace BlackMagic;

template<typename T>
T ReadType(BlackMagic::byte*& data)
{
	T x = *(T*)data;
	data += sizeof(T);
	return x;
}

WAVFile::WAVFile(AudioFile audioFile)
{
	this->audioFile = audioFile;
	//Process the audio file
	byte* data = audioFile.GetAs<byte*>();
	int chunkID = 0;
	bool dataChunk = false;
	while (!dataChunk)
	{
		chunkID = ReadType<int>(data);

		if(chunkID == *(int*)"fmt ")
		{
			int formatSize = ReadType<int>(data);
			short format = ReadType<short>(data);
			short channelCount = ReadType<short>(data);
			int sampleRate = ReadType<int>(data);
			int bitsPerSecond = ReadType<int>(data);
			short formatBlockAlign = ReadType<short>(data);
			short bitDepth = ReadType<short>(data);

			if (format != 1)
			{
				assert(false); //bad file, not PCM data
			}

			//If there's extra data, skip the size of the extra data
			if (formatSize == 18)
			{
				int extradata = ReadType<short>(data);
				data += extradata;
			}

			this->channelCount = channelCount;
			this->samplesPerSecond = sampleRate;
			this->bitsPerSample = bitsPerSecond / sampleRate;
		}
		else if(chunkID == *(int*)"RIFF")
		{
			int headerID = chunkID;
			int memSize = ReadType<int>(data);
			int riffStyle = ReadType<int>(data);
			int wavDesc = *(int*)"WAVE";
			if (riffStyle != wavDesc)
			{
				assert(false); // This is not a WAV file!
			}
		}
		else if(chunkID == *(int*)"data")
		{
			dataChunk = true;
			int dataSize = ReadType<int>(data);
			this->pcmData = data;
			this->dataSize = dataSize;
		}
		else
		{
			int skipSize = ReadType<int>(data);
			data += skipSize;
		}
	}
}
