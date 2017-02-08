#include "ThreadManager.h"

using namespace BlackMagic;

ThreadManager::ThreadManager(byte* spaceLocation, size_t spaceSize) :
	allocator((size_t)32, spaceSize, spaceLocation)
{
}


ThreadManager::~ThreadManager()
{
}
