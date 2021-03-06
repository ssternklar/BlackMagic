#include "ContentJob.h"
#include "PlatformBase.h"
#include "ContentManager.h"

using namespace BlackMagic;

ContentJob_Base::ContentJob_Base(const char* resourceName) : resourceLocation(nullptr)
{
	this->resourceName = resourceName;
	resourceLocation = nullptr;
};

void ContentJob_Base::WaitUntilJobIsComplete()
{
	while (!done && !PlatformBase::GetSingleton()->ShouldExit()) {}
};