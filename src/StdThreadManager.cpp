#include "StdThreadManager.h"

using namespace BlackMagic;
using namespace std;

void BlackMagic::StdThreadManager::PlatformCreateThread(InternalThreadWorker worker, ThreadManager * manager)
{
	if (currentThreadCount < MAX_THREADS)
	{
		threads[currentThreadCount] = thread(worker, manager);
		currentThreadCount++;
	}
}

Mutex BlackMagic::StdThreadManager::PlatformCreateMutex()
{
	if (currentMutexCount < MAX_MUTEXES)
	{
		Mutex ret(&mutexes[currentMutexCount]);
		currentMutexCount++;
		return ret;
	}
	throw "No more mutexes remaining, increase mutex pool count";
}

void BlackMagic::StdThreadManager::PlatformLockMutex(Mutex m)
{
	m.GetAs<mutex*>()->lock();
}

void BlackMagic::StdThreadManager::PlatformUnlockMutex(Mutex m)
{
	m.GetAs<mutex*>()->unlock();
}

void BlackMagic::StdThreadManager::PlatformSleepThisThread(unsigned int s)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(s));
}

StdThreadManager::StdThreadManager(PlatformBase* base, BlackMagic::byte* spaceLocation, size_t spaceSize) : ThreadManager(base, spaceLocation, spaceSize)
{
	allocatorMutex = PlatformCreateMutex();
	GenericTaskListMutex = PlatformCreateMutex();
	RenderTaskListMutex = PlatformCreateMutex();
	ContentTaskListMutex = PlatformCreateMutex();
	AudioTaskListMutex = PlatformCreateMutex();
}


StdThreadManager::~StdThreadManager()
{
	for (int i = 0; i < currentThreadCount; i++)
	{
		threads[i].join();
	}
}
