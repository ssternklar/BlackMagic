#include "PlatformBase.h"
#ifdef _WIN32
#include <new>
#include <Windows.h>
#endif

using namespace BlackMagic;

const int CPU_MEMORY_SIZE = 1024 * 1024 * 1024;

PlatformBase* PlatformBase::singleton = nullptr;

void PlatformBase::GetScreenDimensions(unsigned int* width, unsigned int* height)
{
	if (width != nullptr) *width = windowWidth;
	if (height != nullptr) *height = windowHeight;
}

bool PlatformBase::BlackMagicInit()
{
	singleton = this;
	byte* ptr;
	GetSystemMemory(CPU_MEMORY_SIZE, &ptr);
	allocatorAllocator = new (ptr) StackAllocator(32, CPU_MEMORY_SIZE);
	TheCPUMemory = ptr;

	byte* contentMemory = (byte*)allocatorAllocator->allocate(1024 * 1024 * 256);
	BestFitAllocator* contentAllocator = new (contentMemory) BestFitAllocator(32, 1024 * 1024 * 256);

	contentManager = allocatorAllocator->allocate<ContentManager>();

	InitWindow();
	InitPlatformRenderer();

	contentManager = new (contentManager) ContentManager(
		renderer,
		GetAssetDirectory(),
		contentAllocator
	);
	
	
	//Content manager manifest setup
	char path[256];
	memset(path, 0, 256);
	strcpy_s(path, GetAssetDirectory());
	strcat_s(path, "manifest.bm");
	unsigned int fileSize = GetFileSize(path);
	BlackMagic::byte* manifestFile = (byte*)allocatorAllocator->allocate(fileSize, 1);
	ReadFileIntoMemory(path, manifestFile, fileSize);
	contentManager->ProcessManifestFile(manifestFile);
	allocatorAllocator->deallocate(manifestFile, fileSize, 1);

	renderer->Init(contentManager);
	InitPlatformAudioManager();
	InitPlatformThreadManager();

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
	
	if (renderer)
		renderer->~Renderer();
	
	if (threadManager)
		threadManager->~ThreadManager();
	
	if (audioManager)
		audioManager->~AudioManager();

	ReturnSystemMemory(TheCPUMemory);
}

InputData* PlatformBase::GetInputData()
{
	return &inputData;
}

BlackMagic::Renderer* BlackMagic::PlatformBase::GetRenderer()
{
	return renderer;
}

ContentManager* BlackMagic::PlatformBase::GetContentManager()
{
	return contentManager;
}

ThreadManager* BlackMagic::PlatformBase::GetThreadManager()
{
	return threadManager;
}

AudioManager* BlackMagic::PlatformBase::GetAudioManager()
{
	return audioManager;
}

void BlackMagic::PlatformBase::GetGameMemory(byte** gameMemoryStorage, size_t* gameMemorySizeStorage)
{
	*gameMemoryStorage = gameMemory;
	*gameMemorySizeStorage = gameMemorySize;
}

PlatformBase* BlackMagic::PlatformBase::GetSingleton()
{
	return singleton;
}
