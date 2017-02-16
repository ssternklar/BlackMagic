#pragma once
#include "ThreadManager.h"
#include <thread>
#include <mutex>

namespace BlackMagic
{
	class StdThreadManager : public ThreadManager
	{
		static const int MAX_THREADS = 8;
		static const int MAX_MUTEXES = 32;
		int currentThreadCount = 0;
		int currentMutexCount = 0;
	protected:
		virtual void PlatformCreateThread(InternalThreadWorker worker, ThreadManager* manager) override;
		virtual Mutex PlatformCreateMutex() override;
		virtual void PlatformLockMutex(Mutex mutex) override;
		virtual void PlatformUnlockMutex(Mutex mutex) override;
	public:
		std::thread threads[MAX_THREADS];
		std::mutex mutexes[MAX_MUTEXES];
		StdThreadManager(PlatformBase* base, byte* spaceLocation, size_t spaceSize);
		~StdThreadManager();
	};
}