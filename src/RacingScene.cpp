#include "RacingScene.h"
#include "SceneBasedGame.h"
#include "ContentManager.h"

using namespace BlackMagic;

SceneBasedGame<RacingScene>* SceneBasedGame<RacingScene>::singleton = nullptr;

RacingScene::RacingScene(BlackMagic::BestFitAllocator* allocator) : BlackMagic::Scene(allocator), entities(AllocatorAdapter<Entity*>(allocator))
{
}


RacingScene::~RacingScene()
{
}

void RacingScene::Update(float deltaTime)
{
	for (Entity* entity : entities)
	{
		entity->Update(deltaTime);
	}
}

void RacingScene::Draw(float deltaTime)
{
}

enum class SceneTags
{
	CAMERA = 0,
	ARBITRARY = 1,
	PLAY_EASY = 2,
	PLAY_MEDIUM = 3,
	PLAY_HARD = 4,
	QUIT = 5,
	CURSOR = 6,
};

void RacingScene::ProcessType(uint16_t tag, Transform transform, BlackMagic::AssetPointer<BlackMagic::Mesh> mesh, BlackMagic::AssetPointer<BlackMagic::Material> material)
{
	switch ((SceneTags)tag)
	{
	case SceneTags::CAMERA:
		camera = AllocateAndConstruct<Camera>(alloc, 1, CreateVector3(0,-1,-2));
		camera->Update(transform);
		break;
	case SceneTags::ARBITRARY:
		entities.push_back(AllocateAndConstruct<Entity>(alloc, 1, transform.GetPosition(), transform.GetRotation(), PlatformBase::GetSingleton()->GetContentManager()->ConvertToSharedPtr(mesh), *material));
		break;
	case SceneTags::PLAY_EASY:
		menuPositions[0] = transform;
		break;
	case SceneTags::PLAY_MEDIUM:
		menuPositions[1] = transform;
		break;
	case SceneTags::PLAY_HARD:
		menuPositions[2] = transform;
		break;
	case SceneTags::QUIT:
		menuPositions[3] = transform;
		break;
	case SceneTags::CURSOR:
		cursor = AllocateAndConstruct<MenuCursor>(alloc, 1, transform, mesh, material, menuPositions);
		entities.push_back(cursor);
		break;
		
	}
}