#include "TestJob.h"
#include <cstdio>

using namespace BlackMagic;

TestJob::TestJob()
{
}


TestJob::~TestJob()
{
}

void BlackMagic::TestJob::Run()
{
	for (int i = 0; i < 100; i++)
	{
		std::printf("%d\n", i);
	}
}
