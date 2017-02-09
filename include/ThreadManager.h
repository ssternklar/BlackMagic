#pragma once

#include "allocators\globals.h"
#include "allocators\BadBestFitAllocator.h"
#include "GenericJob.h"
#include "ContentJob.h"
#include "RenderJob.h"

namespace BlackMagic {

	class PlatformBase;

	class ThreadManager
	{
	protected:
		
		void* managedSpace;
		PlatformBase* base;
		size_t managedSpaceSize;
		BestFitAllocator allocator;

		typedef void(*InternalThreadWorker)(ThreadManager*);

		virtual void PlatformCreateThread(InternalThreadWorker worker, ThreadManager* manager) = 0;
	public:
		//These are public because lambdas
		void RunGenericWorker();
		void RunRenderWorker();
		void RunContentWorker();

		//There can be as many generic threads as you want there to be
		void CreateGenericThread();

		//There can only be one Render thread
		void CreateRenderThread();

		//There can only be one IO thread
		void CreateContentThread();

		template<class JobType, typename... Args>
		JobType* CreateJob(Args&&... args);

		ThreadManager(byte* spaceLocation, size_t spaceSize);
		~ThreadManager();

	};

}