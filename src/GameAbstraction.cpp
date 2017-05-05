#include "GameAbstraction.h"

using namespace BlackMagic;

int BlackMagic::GameAbstraction::RunGame()
{
	PlatformBase* platform = PlatformBase::GetSingleton();
	byte* gameMemory = platform->GetGameMemory();
	size_t memSize = platform->GetGameMemorySize();
	Init(gameMemory, memSize);
	while (!platform->ShouldExit() && !shouldExit)
	{
		platform->InputUpdate();
		float dt = platform->GetDeltaTime();
		Update(dt);
		Draw(dt);
	}
	Destroy();
	return 0;
}