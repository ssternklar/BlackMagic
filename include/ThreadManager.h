#pragma once

#include "allocators\globals.h"
#include "allocators\BadBestFitAllocator.h"
#include "GenericJob.h"
#include "ContentJob.h"
#include "RenderJob.h"
#include "AudioJob.h"
#include "LinkedList.h"
#include "Handles.h"

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
		virtual void PlatformSleepThisThread(unsigned int t) = 0;

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
			JobType* job = AllocateAndConstruct<JobType, BestFitAllocator, Args...>(&allocator, 1, std::forward<Args>(args)...);
			if (job)
			{
				LinkedList* next = AllocateAndConstruct<LinkedList, BestFitAllocator, JobType*>(&allocator, 1, job);
				PlatformUnlockMutex(allocatorMutex);
				PlatformLockMutex(GenericTaskListMutex);
				if (GenericTaskList == nullptr)
				{
					GenericTaskList = next;
				}
				else
				{
					LinkedList* nxt = ContentTaskList;
					while (nxt && nxt->next)
					{
						nxt = nxt->next;
					}
					nxt->next = next;
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
				DestructAndDeallocate<LinkedList>(&allocator, node, 1);
			}
			DestructAndDeallocate<JobType>(&allocator, job, 1);
			PlatformUnlockMutex(allocatorMutex);
		}

		RenderJob* CreateRenderJob();
		void DestroyRenderJob(RenderJob* job);

		AudioJob* CreateAudioJob(bool isBGM, WAVFile* file, float relativeVolume, byte type);

		template<class T>
		ContentJob<T>* CreateContentJob(const char* resourceName)
		{
			PlatformLockMutex(allocatorMutex);
			ContentJob<T>* job = AllocateAndConstruct<ContentJob<T>>(&allocator, 1, resourceName);
			if (job)
			{
				LinkedList* next = AllocateAndConstruct<LinkedList>(&allocator, 1, job);
				PlatformUnlockMutex(allocatorMutex);
				PlatformLockMutex(ContentTaskListMutex);
				if (ContentTaskList == nullptr)
				{
					ContentTaskList = next;
				}
				else
				{
					LinkedList* nxt = ContentTaskList;
					while (nxt && nxt->next)
					{
						nxt = nxt->next;
					}
					nxt->next = next;
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

		void SleepThisThread(unsigned int t)
		{
			PlatformSleepThisThread(t);
		}

		ThreadManager(PlatformBase* base, byte* spaceLocation, size_t spaceSize);
		virtual ~ThreadManager() {};

	};

}