#pragma once
#include "allocators/BadBestFitAllocator.h"
#include "ContentClasses.h"
#include "Scene.h"
#include "GameAbstraction.h"

template<typename SceneType>
class SceneBasedGame : public BlackMagic::GameAbstraction
{
	BlackMagic::BestFitAllocator* gameAllocator;
	SceneType* currentScene;
	BlackMagic::ContentJob<BlackMagic::SceneDesc>* nextScene;
	const char* initialSceneName;
public:
	SceneBasedGame(const char* initialSceneToLoad)
	{
		initialSceneName = initialScene;
	}

	void StartSceneLoad(const char* sceneName)
	{
		BlackMagic::PlatformBase::GetSingleton()->GetThreadManager()->CreateContentJob<BlackMagic::Scene>(sceneName);
	}

	void SwapScene()
	{
		auto sceneDesc = nextScene->GetResult();
		auto prevScene = currentScene;
		auto platformBase = BlackMagic::PlatformBase::GetSingleton();
		currentScene = AllocateAndConstruct<SceneType>(gameAllocator, 1);
		currentScene.Init(gameAllocator, sceneDesc);
		platformBase->GetThreadManager()->DestroyContentJob(nextScene);
		platformBase->GetContentManager()->ForceAssetCleanup(sceneDesc);
		nextScene = nullptr;
		DestructAndDeallocate<SceneType>(gameAllocator, prevScene);
	}

	virtual void Init(byte* gameMemory, size_t memorySize)
	{
		gameAllocator = new (gameMemory) BlackMagic::BestFitAllocator(16, memorySize);
		StartSceneLoad(initialSceneName);
		SwapScene<SceneType>();
	}

	virtual void Update(float deltaTime)
	{
		if (nextScene->done)
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