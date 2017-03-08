#pragma once

#define _BM_HANDLE_TYPE_INFO(X)\
void* handleInternal; \
public:\
X##() : handleInternal(nullptr) {} \
X##(void* ptr) : handleInternal(ptr) {} \
\
template<typename T> \
T GetAs(){return static_cast<T>(handleInternal);}

namespace BlackMagic
{
	class Mutex
	{
		_BM_HANDLE_TYPE_INFO(Mutex)
	};

	class AudioFile
	{
		_BM_HANDLE_TYPE_INFO(AudioFile)
	};

	class AudioChannel
	{
		_BM_HANDLE_TYPE_INFO(AudioChannel)
	};
}

#undef _BM_HANDLE_TYPE_INFO(X)