#pragma once

#include "allocators\globals.h"
#include "allocators\BadBestFitAllocator.h"

namespace BlackMagic {

	//Runs some asynchronous code in a Generic thread
	class GenericJob
	{
	public:
		virtual void Run();
	};


	// Submits a draw call
	class RenderJob
	{
	};

	//Promise/Future style content load job
	class ContentJob
	{
	public:
		void WaitUntilJobIsComplete();
	};

	class ThreadManager
	{
	private:

		void* managedSpace;
		size_t managedSpaceSize;
		BestFitAllocator allocator;
		
		typedef void(*InternalThreadWorker)();
		virtual void PlatformCreateThread(InternalThreadWorker workerFunction) = 0;
		void internal_GenericWorker();
		void internal_RenderWorker();
		void internal_ContentWorker();
	public:
		typedef void(*GenericWorkerThreadFunc)(GenericJob*);
		typedef void(*RenderWorkerThreadFunc)(RenderJob*);
		typedef void(*ContentWorkerThreadFunc)(ContentJob*);

		//There can be as many generic threads as you want there to be
		void CreateGenericThread(GenericWorkerThreadFunc function);

		//There can only be one Render thread
		void CreateRenderThread(RenderWorkerThreadFunc function);

		//There can only be one Content thread
		void CreateContentLoaderThread(ContentWorkerThreadFunc function);
		ThreadManager(byte* spaceLocation, size_t spaceSize);
		~ThreadManager();

	};

}