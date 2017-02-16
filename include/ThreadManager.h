#pragma once

#include "allocators\globals.h"
#include "allocators\BadBestFitAllocator.h"
#include "GenericJob.h"
#include "ContentJob.h"
#include "RenderJob.h"
#include "LinkedList.h"
#include "Handles.h"
#include <atomic>

namespace BlackMagic {

	class PlatformBase;

	class ThreadManager
	{
	protected:
		
		enum ThreadType
		{
			GENERIC,
			RENDER,
			CONTENT
		};

		void* managedSpace;
		PlatformBase* base;
		size_t managedSpaceSize;
		BestFitAllocator allocator;

		Mutex allocatorMutex;
		Mutex GenericTaskListMutex;
		Mutex RenderTaskListMutex;
		Mutex ContentTaskListMutex;

		LinkedList* GenericTaskList = nullptr;
		LinkedList* RenderTaskList = nullptr;
		LinkedList* ContentTaskList = nullptr;
		
		typedef void(*InternalThreadWorker)(ThreadManager*);

		virtual void PlatformCreateThread(InternalThreadWorker worker, ThreadManager* manager) = 0;
		virtual Mutex PlatformCreateMutex() = 0;
		virtual void PlatformLockMutex(Mutex mutex) = 0;
		virtual void PlatformUnlockMutex(Mutex mutex) = 0;


	public:

		//These are public because lambdas
		void RunGenericWorker();
		void RunRenderWorker();
		void RunContentWorker();

		//There can be as many generic threads as you want there to be
		void CreateGenericThread();

		//There can only be one Render thread as of right now
		void CreateRenderThread();

		//There *should* only be one content thread as of right now
		void CreateContentThread();

		template<class JobType, typename... Args>
		JobType* CreateGenericJob(Args&&... args);
		template<class JobType>
		void DestroyGenericJob(JobType* job);

		RenderJob* CreateRenderJob();
		void DestroyRenderJob(RenderJob* job);

		template<class T>
		ContentJob<T>* CreateContentJob(char* resourceName);
		template<class T>
		void DestroyContentJob(ContentJob<T>* job);


		ThreadManager(PlatformBase* base, byte* spaceLocation, size_t spaceSize);
		~ThreadManager();

	};

}