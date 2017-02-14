#pragma once
namespace BlackMagic
{
	//Promise/Future style generic job that can be extended
	//Return type of job should be done via a GetResult function of some kind
	class GenericJob
	{
		friend class ThreadManager;
	protected:
		bool inProgress = false;
		virtual void Run() = 0;
		bool done = false;
	public:
		void WaitUntilJobIsComplete()
		{
			while (!done) {}
		};
	};
}