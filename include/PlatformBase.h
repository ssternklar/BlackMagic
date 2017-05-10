#pragma once
#include "AudioManager.h"
#include "allocators\globals.h"
#include "allocators\StackAllocator.h"
#include "allocators\BadBestFitAllocator.h"
#include "Renderer.h"
#include "InputData.h"
#include "Transform.h"

namespace BlackMagic {

	class ContentManager;
	class ThreadManager;

	class PlatformBase
	{
	protected:
		InputData inputData;
		StackAllocator* allocatorAllocator;
		byte* gameMemory;
		size_t gameMemorySize;
		byte* TheCPUMemory;
		Renderer* renderer;
		ContentManager* contentManager;
		ThreadManager* threadManager;
		AudioManager* audioManager;
		unsigned int windowWidth = 1280;
		unsigned int windowHeight = 720;
		bool manualExitTriggered = false;
		static PlatformBase* singleton;
	public:
		static const int CPU_MEMORY_SIZE = 1024 * 1024 * 1024;

		virtual bool InitWindow() = 0;
		virtual void InitPlatformAudioManager() = 0;
		virtual void InitPlatformThreadManager() = 0;
		virtual void InitPlatformRenderer() = 0;
		virtual bool GetSystemMemory(size_t memSize, byte** ptrToRetrievedMemory) = 0;
		virtual void InputUpdate() = 0;
		virtual bool ShouldExit() = 0;
		virtual float GetDeltaTime() = 0;
		virtual void ReturnSystemMemory(byte* memory) = 0;
		virtual const char* GetAssetDirectory() = 0;
		virtual bool ReadFileIntoMemory(const char* fileName, byte* fileBuffer, size_t bufferSize) = 0;
		virtual unsigned int GetFileSize(const char* fileName) = 0;
		virtual void ShutdownPlatform() = 0;
		void SetScreenDimensions(unsigned int width, unsigned int height);
		void GetScreenDimensions(unsigned int* width, unsigned int* height);
		bool BlackMagicInit();
		void BlackMagicCleanup();
		InputData* GetInputData();
		Renderer* GetRenderer();
		ContentManager* GetContentManager();
		ThreadManager* GetThreadManager();
		AudioManager* GetAudioManager();
		byte* GetGameMemory();
		size_t GetGameMemorySize();
		static PlatformBase* GetSingleton();
		void Exit();
	};
}