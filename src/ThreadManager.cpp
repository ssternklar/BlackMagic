#include "ThreadManager.h"
#include "PlatformBase.h"

using namespace BlackMagic;

//These are public because lambdas
void internal_GenericWorker(ThreadManager* manager)
{
	manager->RunGenericWorker();
}

void internal_RenderWorker(ThreadManager* manager)
{
	manager->RunRenderWorker();
}
void internal_ContentWorker(ThreadManager* manager)
{
	manager->RunContentWorker();
}

void internal_AudioWorker(ThreadManager* manager)
{
	manager->RunAudioWorker();
}

ThreadManager::ThreadManager(PlatformBase* base, BlackMagic::byte* spaceLocation, size_t spaceSize) :
	allocator((size_t)32, spaceSize, spaceLocation)
{
	this->base = base;
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

void BlackMagic::ThreadManager::CreateAudioThread()
{
	PlatformCreateThread(internal_AudioWorker, this);
}

void ThreadManager::RunGenericWorker()
{
	while (!base->ShouldExit())
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
			job->done = true;
			if (job->cleanupSelf)
				DestroyGenericJob<GenericJob>(job);
		}
		else
		{
			PlatformSleepThisThread(16);
		}
	}
}

void ThreadManager::RunRenderWorker()
{
	while (!base->ShouldExit())
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
		else
		{
			PlatformSleepThisThread(16);
		}
	}
}

void ThreadManager::RunContentWorker()
{
	while (!base->ShouldExit())
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
			job->Run();
			job->done = true;
		}
		else
		{
			PlatformSleepThisThread(16);
		}
	}
}

void BlackMagic::ThreadManager::RunAudioWorker()
{
	while (!base->ShouldExit())
	{
		while(AudioTaskList != nullptr)
		{
			LinkedList* toDelete = nullptr;
			AudioJob* job = nullptr;
			PlatformLockMutex(AudioTaskListMutex);
			if (AudioTaskList)
			{
				job = AudioTaskList->GetAs<AudioJob>();
				toDelete = AudioTaskList;
				AudioTaskList = AudioTaskList->next;
			}
			PlatformUnlockMutex(AudioTaskListMutex);
			if (toDelete)
			{
				PlatformLockMutex(allocatorMutex);
				DestructAndDeallocate(&allocator, toDelete, 1);
				PlatformUnlockMutex(allocatorMutex);
				if (AudioTaskList == toDelete)
				{
					AudioTaskList = nullptr;
				}
			}
			if (job)
			{
				//Run the job
				job->Run();
				PlatformLockMutex(allocatorMutex);
				DestructAndDeallocate(&allocator, job, 1);
				PlatformUnlockMutex(allocatorMutex);
			}
		}
		PlatformBase::GetSingleton()->GetAudioManager()->UpdateAudio();
	}
}

RenderJob* ThreadManager::CreateRenderJob()
{
	PlatformLockMutex(allocatorMutex);
	RenderJob* job = AllocateAndConstruct<RenderJob>(&allocator, 1);
	if (job)
	{
		LinkedList* next = AllocateAndConstruct<LinkedList>(&allocator, 1, job);
		PlatformUnlockMutex(allocatorMutex);
		PlatformLockMutex(RenderTaskListMutex);
		if (RenderTaskList == nullptr)
		{
			RenderTaskList = next;
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

AudioJob* BlackMagic::ThreadManager::CreateAudioJob(bool isBGM, WAVFile* file, float relativeVolume, byte status)
{
	PlatformLockMutex(allocatorMutex);
	AudioJob* job = AllocateAndConstruct<AudioJob>(&allocator, 1);
	job->fileToPlay = file;
	job->relativeVolume = relativeVolume;
	job->isBGM = isBGM;
	job->bgmPlayPauseStopResume = status;
	if (job)
	{
		LinkedList* next = AllocateAndConstruct<LinkedList>(&allocator, 1, job);
		PlatformUnlockMutex(allocatorMutex);
		PlatformLockMutex(AudioTaskListMutex);
		if (AudioTaskList == nullptr)
		{
			AudioTaskList = next;
		}
		else
		{
			LinkedList* nxt = AudioTaskList;
			while (nxt && nxt->next)
			{
				nxt = nxt->next;
			}
			nxt->next = next;

		}
		PlatformUnlockMutex(AudioTaskListMutex);
	}
	else
	{
		PlatformUnlockMutex(allocatorMutex);
	}
	return job;
}