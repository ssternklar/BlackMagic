#pragma once
#include "allocators/globals.h"
#include "allocators/BadBestFitAllocator.h"
#include "ContentClasses.h"
#include "ThreadManager.h"
#include "Scene.h"
#include "GameAbstraction.h"
#include "ContentJob.h"

template<typename SceneType>
class SceneBasedGame : public BlackMagic::GameAbstraction
{
	static SceneBasedGame* singleton;

	BlackMagic::BestFitAllocator* gameAllocator;
	SceneType* currentScene;
	BlackMagic::ContentJob<BlackMagic::SceneDesc>* nextScene;
	const char* initialSceneName;
public:
	SceneBasedGame(const char* initialSceneToLoad)
	{
		initialSceneName = initialSceneToLoad;
		singleton = this;
	}

	static SceneBasedGame* GetSingleton()
	{
		return singleton;
	}

	void StartSceneLoad(const char* sceneName)
	{
		nextScene = BlackMagic::PlatformBase::GetSingleton()->GetThreadManager()->CreateContentJob<BlackMagic::SceneDesc>(sceneName);
	}

	void SwapScene()
	{
		if (nextScene)
		{
			nextScene->WaitUntilJobIsComplete();
			auto sceneDesc = nextScene->GetResult();
			SceneType* prevScene = currentScene;
			auto platformBase = BlackMagic::PlatformBase::GetSingleton();
			currentScene = BlackMagic::AllocateAndConstruct<SceneType>(gameAllocator, 1, gameAllocator);
			currentScene->Init(sceneDesc);
			currentScene->Start();
			platformBase->GetThreadManager()->DestroyContentJob(nextScene);
			platformBase->GetContentManager()->ForceAssetCleanup(sceneDesc.entry);
			nextScene = nullptr;
			if(prevScene)
			{
				BlackMagic::DestructAndDeallocate<SceneType>(gameAllocator, prevScene, 1);
			}
		}
	}

	virtual void Init(BlackMagic::byte* gameMemory, size_t memorySize)
	{
		gameAllocator = new (gameMemory) BlackMagic::BestFitAllocator(16, memorySize);
		StartSceneLoad(initialSceneName);
		SwapScene();
	}

	virtual void Update(float deltaTime)
	{
		if (nextScene && nextScene->done)
		{
			SwapScene();
		}
		currentScene->Update(deltaTime);
	}

	virtual void Draw(float deltaTime)
	{
		currentScene->Draw(deltaTime);
	}
	virtual void Destroy()
	{

	}
};