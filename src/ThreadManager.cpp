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
		GenericJob* job;//Get next job from list
		job->Run(); //run the job
	}
}

void ThreadManager::RunRenderWorker()
{
	while (true)
	{
		RenderJob* job; // try to get next job from queue
		//Render stuff
	}
}

void ThreadManager::RunContentWorker()
{
	while (true)
	{
		ContentJob_Base* job;
		//base->GetContentManager()->Load("Thing")
	}
}

template<class JobType, typename... Args>
JobType* ThreadManager::CreateJob(Args&&... args)
{
	//Generic Job Here
	return nullptr;
}

template<>
RenderJob* ThreadManager::CreateJob()
{
	//Render Job Here
	return nullptr;
}

template<>
ContentJob_Base* ThreadManager::CreateJob(char* resourceName)
{
	//ContentJobHere
	return nullptr;
}

ThreadManager::~ThreadManager()
{
}
