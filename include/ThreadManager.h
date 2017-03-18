#pragma once

#include "allocators\globals.h"
#include "allocators\BadBestFitAllocator.h"
#include "GenericJob.h"
#include "ContentJob.h"
#include "RenderJob.h"
#include "AudioJob.h"
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
		Mutex AudioTaskListMutex;

		LinkedList* GenericTaskList = nullptr;
		LinkedList* RenderTaskList = nullptr;
		LinkedList* ContentTaskList = nullptr;
		LinkedList* AudioTaskList = nullptr;

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
		void RunAudioWorker();

		//There can be as many generic threads as you want there to be
		void CreateGenericThread();

		//There can only be one Render thread as of right now
		void CreateRenderThread();

		//There *should* only be one content thread as of right now
		void CreateContentThread();

		//There *should* only be one audio thread as of right now
		void CreateAudioThread();

		template<class JobType, typename... Args>
		JobType* CreateGenericJob(Args&&... args)
		{
			PlatformLockMutex(allocatorMutex);
			JobType* job = AllocateAndConstruct<BestFitAllocator, JobType, Args...>(&allocator, 1, args...);
			if (job)
			{
				LinkedList* next = AllocateAndConstruct<BestFitAllocator, LinkedList, JobType*>(&allocator, 1, job);
				PlatformUnlockMutex(allocatorMutex);
				PlatformLockMutex(GenericTaskListMutex);
				if (GenericTaskList == nullptr)
				{
					GenericTaskList = next;
				}
				else
				{
					GenericTaskList->next = next;
				}
				PlatformUnlockMutex(GenericTaskListMutex);
			}
			else
			{
				PlatformUnlockMutex(allocatorMutex);
			}
			return job;
		}

		template<class JobType>
		void DestroyGenericJob(JobType* job)
		{
			LinkedList* node = nullptr;
			PlatformLockMutex(GenericTaskListMutex);
			if (!job->inProgress && !job->done)
			{
				//in the list, find the node
			}
			PlatformUnlockMutex(GenericTaskListMutex);
			PlatformLockMutex(allocatorMutex);
			if (node)
			{
				DestructAndDeallocate<BestFitAllocator, LinkedList>(&allocator, node, 1);
			}
			DestructAndDeallocate<BestFitAllocator, JobType>(&allocator, job, 1);
			PlatformUnlockMutex(allocatorMutex);
		}

		RenderJob* CreateRenderJob();
		void DestroyRenderJob(RenderJob* job);

		AudioJob* CreatePlayAudioJob(bool isBGM, AudioFile file, float relativeVolume);
		AudioJob* CreateStopBGMAudioJob();

		template<class T>
		ContentJob<T>* CreateContentJob(char* resourceName)
		{
			PlatformLockMutex(allocatorMutex);
			ContentJob<T>* job = AllocateAndConstruct(&allocator, 1, resourceName);
			if (job)
			{
				LinkedList* next = AllocateAndConstruct(&allocator, 1, job);
				PlatformUnlockMutex(allocatorMutex);
				PlatformLockMutex(ContentTaskListMutex);
				if (ContentTaskList == nullptr)
				{
					ContentTaskList = next;
				}
				else
				{
					ContentTaskList->next = next;
				}
				PlatformUnlockMutex(ContentTaskListMutex);
			}
			else
			{
				PlatformUnlockMutex(allocatorMutex);
			}
			return job;
		}

		template<class T>
		void DestroyContentJob(ContentJob<T>* job)
		{
			LinkedList* node = nullptr;
			PlatformLockMutex(ContentTaskListMutex);
			if (!job->inProgress && !job->done)
			{
				//in the list, find the node
			}
			PlatformUnlockMutex(ContentTaskListMutex);
			PlatformLockMutex(allocatorMutex);
			if (node)
			{
				DestructAndDeallocate(&allocator, node, 1);
			}
			DestructAndDeallocate(&allocator, job, 1);
			PlatformUnlockMutex(allocatorMutex);
		}

		ThreadManager(PlatformBase* base, byte* spaceLocation, size_t spaceSize);
		virtual ~ThreadManager() {};

	};

}