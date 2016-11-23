#include "GameAbstraction.h"

using namespace BlackMagic;

GameAbstraction::GameAbstraction(PlatformBase* platformBase)
{
	platform = platformBase;
}

int BlackMagic::GameAbstraction::RunGame()
{
	byte* gameMemory;
	size_t memSize;
	platform->GetGameMemory(&gameMemory, &memSize);
	Init(gameMemory, memSize);
	while (!platform->ShouldExit())
	{
		platform->InputUpdate();
		float dt = platform->GetDeltaTime();
		Update(dt);
		Draw(dt);
	}
	return 0;
}