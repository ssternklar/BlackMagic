#pragma once
#include "GenericJob.h"

namespace BlackMagic
{
	class TestJob : public GenericJob
	{
	public:
		TestJob();
		~TestJob();
		void Run() override;
	};
}