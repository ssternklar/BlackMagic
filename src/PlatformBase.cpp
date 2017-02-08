#include "PlatformBase.h"
#ifdef _WIN32
#include <new>
#include <Windows.h>
#endif

using namespace BlackMagic;

const int CPU_MEMORY_SIZE = 1024 * 1024 * 1024;

void PlatformBase::GetScreenDimensions(unsigned int* width, unsigned int* height)
{
	if (width != nullptr) *width = windowWidth;
	if (height != nullptr) *height = windowHeight;
}

bool PlatformBase::BlackMagicInit()
{
	byte* ptr;
	GetSystemMemory(CPU_MEMORY_SIZE, &ptr);
	allocatorAllocator = new (ptr) StackAllocator(32, CPU_MEMORY_SIZE);
	TheCPUMemory = ptr;

	byte* contentMemory = (byte*)allocatorAllocator->allocate(1024 * 1024 * 512);
	BestFitAllocator* contentAllocator = new (contentMemory) BestFitAllocator(32, 1024 * 1024 * 512);

	contentManager = allocatorAllocator->allocate<ContentManager>(false);
	InitWindow();
	InitPlatformGraphicsDevice();

	contentManager = new (contentManager) ContentManager(
		graphicsDevice,
		L"./assets/",
		contentAllocator
	);
	
	graphicsDevice->Init(contentManager);

	transformData = allocatorAllocator->allocate<TransformData>();
	new (transformData) TransformData;
	gameMemorySize = allocatorAllocator->GetRemainingSize() - 32;
	gameMemory = (byte*)allocatorAllocator->allocate(gameMemorySize);

	return true;
}

void PlatformBase::BlackMagicCleanup()
{
	if (contentManager)
		contentManager->~ContentManager();
	
	if (graphicsDevice)
		graphicsDevice->~GraphicsDevice();
	
	ReturnSystemMemory(TheCPUMemory);
}

InputData* PlatformBase::GetInputData()
{
	return &inputData;
}

BlackMagic::GraphicsDevice* BlackMagic::PlatformBase::GetGraphicsDevice()
{
	return graphicsDevice;
}

ContentManager* BlackMagic::PlatformBase::GetContentManager()
{
	return contentManager;
}

void BlackMagic::PlatformBase::GetGameMemory(byte** gameMemoryStorage, size_t* gameMemorySizeStorage)
{
	*gameMemoryStorage = gameMemory;
	*gameMemorySizeStorage = gameMemorySize;
}