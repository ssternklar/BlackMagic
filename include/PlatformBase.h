#include "allocators\globals.h"
#include "allocators\StackAllocator.h"
#include "allocators\BadBestFitAllocator.h"
#include "GraphicsDevice.h"
#include "ContentManager.h"
#include "InputData.h"
#include "TransformData.h"
#include "Transform.h"
#pragma once

namespace BlackMagic {

	class PlatformBase
	{
	protected:
		InputData inputData;
		StackAllocator* allocatorAllocator;
		byte* gameMemory;
		size_t gameMemorySize;
		byte* TheCPUMemory;
		GraphicsDevice* graphicsDevice;
		ContentManager* contentManager;
		TransformData* transformData;
		int windowWidth = 1280;
		int windowHeight = 720;
	public:
		virtual bool InitWindow() = 0;
		virtual void InitPlatformGraphicsDevice() = 0;
		virtual bool GetSystemMemory(size_t memSize, byte** ptrToRetrievedMemory) = 0;
		virtual void InputUpdate() = 0;
		virtual bool ShouldExit() = 0;
		virtual float GetDeltaTime() = 0;
		void GetScreenDimensions(unsigned int* width, unsigned int* height);
		bool BlackMagicInit();
		InputData* GetInputData();
		GraphicsDevice* GetGraphicsDevice();
		ContentManager* GetContentManager();
		void GetGameMemory(byte** gameMemoryStorage, size_t* gameMemorySizeStorage);
	};
}