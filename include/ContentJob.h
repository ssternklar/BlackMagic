#pragma once

#include "ContentManager.h"

namespace BlackMagic
{
	class ContentJob_Base
	{
		friend class ThreadManager;
	protected:
		void* resourceLocation;
		bool inProgress;
		bool done;
	public:
		char* resourceName;
		ContentJob_Base(char* resourceName);
		void WaitUntilJobIsComplete();
		virtual void Run() = 0;
	};

	//Promise/Future style content load job
	template<typename T>
	class ContentJob : public ContentJob_Base
	{
	public:
		T* GetResult() { return static_cast<T>(resourceLocation) };
		virtual void Run() override;
	};
}