#pragma once
#include "ContentClasses.h"
namespace BlackMagic
{
	class ContentJob_Base
	{
		friend class ThreadManager;
	protected:
		AssetPointer_Base resourceLocation;
		bool inProgress;
	public:
		bool done;
		const char* resourceName;
		ContentJob_Base(const char* resourceName);
		void WaitUntilJobIsComplete();
		virtual void Run() = 0;
	};

	//Promise/Future style content load job
	template<typename T>
	class ContentJob : public ContentJob_Base
	{
	public:
		ContentJob(const char* resourceName) : ContentJob_Base(resourceName) {};
		AssetPointer<T> GetResult()
		{
			WaitUntilJobIsComplete();
			return dynamic_cast<AssetPointer<T>>(resourceLocation);
		};
		virtual void Run() override;
	};
}