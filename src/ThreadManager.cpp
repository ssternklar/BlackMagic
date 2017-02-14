#include "ThreadManager.h"
#include "PlatformBase.h"

using namespace BlackMagic;

//These are public because lambdas
void internal_GenericWorker(ThreadManager* manager)
{
	manager->RunGenericWorker();
};
void internal_RenderWorker(ThreadManager* manager)
{
	manager->RunRenderWorker();
};
void internal_ContentWorker(ThreadManager* manager)
{
	manager->RunContentWorker();
};

ThreadManager::ThreadManager(BlackMagic::byte* spaceLocation, size_t spaceSize) :
	allocator((size_t)32, spaceSize, spaceLocation)
{
	allocatorMutex = PlatformCreateMutex();
	GenericTaskListMutex = PlatformCreateMutex();
	RenderTaskListMutex = PlatformCreateMutex();
	ContentTaskListMutex = PlatformCreateMutex();
}

void ThreadManager::CreateGenericThread()
{
	PlatformCreateThread(internal_GenericWorker, this);
}

void ThreadManager::CreateRenderThread()
{
	PlatformCreateThread(internal_RenderWorker, this);
}

void ThreadManager::CreateContentThread()
{
	PlatformCreateThread(internal_ContentWorker, this);
}

void ThreadManager::RunGenericWorker()
{
	while (true)
	{
		LinkedList* toDelete = nullptr;
		GenericJob* job = nullptr;
		PlatformLockMutex(GenericTaskListMutex);
		if(GenericTaskList)
		{
			job = GenericTaskList->GetAs<GenericJob>();
			toDelete = GenericTaskList;
			GenericTaskList = GenericTaskList->next;
		}
		PlatformUnlockMutex(GenericTaskListMutex);
		if (toDelete)
		{
			PlatformLockMutex(allocatorMutex);
			DestructAndDeallocate(&allocator, toDelete, 1);
			PlatformUnlockMutex(allocatorMutex);
		}
		if (job)
		{
			job->Run(); //run the job
		}
	}
}

void ThreadManager::RunRenderWorker()
{
	while (true)
	{
		LinkedList* toDelete = nullptr;
		RenderJob* job = nullptr;
		PlatformLockMutex(RenderTaskListMutex);
		if (RenderTaskList)
		{
			job = RenderTaskList->GetAs<RenderJob>();
			toDelete = RenderTaskList;
			RenderTaskList = RenderTaskList->next;
		}
		PlatformUnlockMutex(RenderTaskListMutex);
		if (toDelete)
		{
			PlatformLockMutex(allocatorMutex);
			DestructAndDeallocate(&allocator, toDelete, 1);
			PlatformUnlockMutex(allocatorMutex);
		}
		if (job)
		{
			//Run the job
			PlatformLockMutex(allocatorMutex);
			DestructAndDeallocate(&allocator, job, 1);
			PlatformUnlockMutex(allocatorMutex);
		}
	}
}

void ThreadManager::RunContentWorker()
{
	while (true)
	{
		LinkedList* toDelete = nullptr;
		ContentJob_Base* job = nullptr;
		PlatformLockMutex(ContentTaskListMutex);
		if (ContentTaskList)
		{
			job = ContentTaskList->GetAs<ContentJob_Base>();
			toDelete = ContentTaskList;
			ContentTaskList = ContentTaskList->next;
		}
		PlatformUnlockMutex(ContentTaskListMutex);
		if (toDelete)
		{
			PlatformLockMutex(allocatorMutex);
			DestructAndDeallocate(&allocator, toDelete, 1);
			PlatformUnlockMutex(allocatorMutex);
		}
		if (job)
		{
			//Run the job
		}
	}
}

template<class JobType, typename... Args>
JobType* ThreadManager::CreateGenericJob(Args&&... args)
{
	PlatformLockMutex(allocatorMutex);
	JobType* job = AllocateAndConstruct(&allocator, 1, args...);
	if (job)
	{
		LinkedList* next = AllocateAndConstruct(&allocator, 1, job);
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
void BlackMagic::ThreadManager::DestroyGenericJob(JobType* job)
{
	LinkedList* node = nullptr;
	PlatformLockMutex(GenericTaskListMutex);
	if (!job->inProgress && !job->done)
	{
		//in the list, find the node
	}
	PlatformUnlockMutex(GenericTaskListMutex);
	PlatformLockMutex(allocatorMutex);
	if(node)
	{
		DestructAndDeallocate(&allocator, node, 1);
	}
	DestructAndDeallocate(&allocator, job, 1);
	PlatformUnlockMutex(allocatorMutex);
}

RenderJob* ThreadManager::CreateRenderJob()
{
	PlatformLockMutex(allocatorMutex);
	RenderJob* job = AllocateAndConstruct(&allocator, 1);
	if (job)
	{
		LinkedList* next = AllocateAndConstruct(&allocator, 1, job);
		PlatformUnlockMutex(allocatorMutex);
		PlatformLockMutex(RenderTaskListMutex);
		if (RenderTaskList == nullptr)
		{
			RenderTaskList = next;
		}
		else
		{
			RenderTaskList->next = next;
		}
		PlatformUnlockMutex(RenderTaskListMutex);
	}
	else
	{
		PlatformUnlockMutex(allocatorMutex);
	}
	return job;
}

void BlackMagic::ThreadManager::DestroyRenderJob(RenderJob * job)
{
}

template<class T>
ContentJob<T>* ThreadManager::CreateContentJob(char* resourceName)
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
void BlackMagic::ThreadManager::DestroyContentJob(ContentJob<T>* job)
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

ThreadManager::~ThreadManager()
{
}
