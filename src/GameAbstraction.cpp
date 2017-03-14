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
	while (!platform->ShouldExit() && !shouldExit)
	{
		platform->InputUpdate();
		float dt = platform->GetDeltaTime();
		Update(dt);
		Draw(dt);

		//To be moved to worker thread soon
		platform->GetAudioManager()->UpdateAudio();
	}
	Destroy();
	return 0;
}