#pragma once


namespace BlackMagic
{
	class ContentJob_Base : public GenericJob
	{
		friend class ThreadManager;
	protected:
		bool inProgress;
		void* resourceLocation;
		bool done;
	public:
		char* resourceName;

		ContentJob_Base(char* resourceName)
		{
			this->resourceName = resourceName;
			resourceLocation = nullptr;
		};

		void WaitUntilJobIsComplete()
		{
			while (!done) {}
		};
	};

	//Promise/Future style content load job
	template<typename T>
	class ContentJob : public ContentJob_Base
	{
	public:
		T* GetResult() { return static_cast<T>(resourceLocation) };
	};
}