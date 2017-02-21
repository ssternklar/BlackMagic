#include "GenericJob.h"
#include "PlatformBase.h"

using namespace BlackMagic;

void GenericJob::WaitUntilJobIsComplete()
{
	while (!done && !PlatformBase::GetSingleton()->ShouldExit()) {}
}