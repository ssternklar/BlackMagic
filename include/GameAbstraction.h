#pragma once
#include "PlatformBase.h"

namespace BlackMagic
{

	class GameAbstraction
	{
	protected:
		PlatformBase* platform;
	public:
		GameAbstraction(PlatformBase* platformBase);
		int RunGame();
		virtual void Init(byte* gameMemory, size_t memorySize) = 0;
		virtual void Update(float deltaTime) = 0;
		virtual void Draw(float deltaTime) = 0;
	};
}