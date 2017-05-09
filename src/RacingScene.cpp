#include "RacingScene.h"
#include "SceneBasedGame.h"
#include "ContentManager.h"

using namespace BlackMagic;

template<> SceneBasedGame<RacingScene>* SceneBasedGame<RacingScene>::singleton = nullptr;

RacingScene::RacingScene(BlackMagic::BestFitAllocator* allocator) : BlackMagic::Scene(allocator), entities(AllocatorAdapter<Entity*>(allocator)), spline(nullptr)
{
	_globalLight = {
		CreateVector4(0.0f, 0.0f, 0.0f, 1.0f),
		CreateVector4(10.0f, 10.0f, 10.0f, 1.0f),
		CreateVector3(1, -1, 1),
		CreateVector3(0, 1, 1)
	};
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

	Transform t;
	if (machine)
	{
		t = machine->GetTransform();
	}
	camera->Update(t);
}

void RacingScene::Draw(float deltaTime)
{
	auto renderer = PlatformBase::GetSingleton()->GetRenderer();
	const Vector4 color{ 0.4f, 0.6f, 0.75f, 0.0f };
	renderer->Clear(color);
	//renderer->Cull(camera, entities, renderables);
	renderer->Render(*camera, entities, _globalLight);
	renderer->Present(0, 0);
}

enum class SceneTags
{
	CAMERA = 0,
	ARBITRARY = 1,
	PLAY_EASY = 2,
	PLAY_MEDIUM = 3, //not used
	PLAY_HARD = 4,   //not used
	QUIT = 5,
	CURSOR = 6,
	TRACK = 7,
	MACHINE = 8,
	LIGHT0 = 9,
	LIGHT1 = 10,
	LIGHT2 = 11,
};

void RacingScene::ProcessType(uint16_t tag, Transform transform, BlackMagic::AssetPointer<BlackMagic::Mesh> mesh, BlackMagic::AssetPointer<BlackMagic::Material> material)
{
	unsigned int width, height;
	PlatformBase::GetSingleton()->GetScreenDimensions(&width, &height);
	switch ((SceneTags)tag)
	{
	case SceneTags::CAMERA:
		camera = AllocateAndConstruct<Camera>(alloc, 1, CreateVector3(0, 1, -2));
		camera->Update(transform);
		camera->UpdateProjectionMatrix(width, height);
		break;
	default:
	case SceneTags::ARBITRARY:
		entities.push_back(AllocateAndConstruct<Entity>(alloc, 1, transform.GetPosition(), transform.GetRotation(), (mesh), *material));
		break;
	case SceneTags::PLAY_EASY:
		menuPositions[0] = transform;
		break;
	case SceneTags::PLAY_MEDIUM:
		//menuPositions[1] = transform;
		//break;
	case SceneTags::PLAY_HARD:
		//menuPositions[2] = transform;
		//break;
	case SceneTags::QUIT:
		menuPositions[1] = transform;
		break;
	case SceneTags::CURSOR:
		cursor = AllocateAndConstruct<MenuCursor>(alloc, 1, transform, mesh, material, menuPositions);
		entities.push_back(cursor);
		break;
	case SceneTags::TRACK:
		entities.push_back(AllocateAndConstruct<Entity>(alloc, 1, transform.GetPosition(), transform.GetRotation(), (mesh), *material));
		spline = PlatformBase::GetSingleton()->GetContentManager()->Load<Spline>("misc/track1.bmspline");
		break;
	case SceneTags::MACHINE:
		machine = AllocateAndConstruct<Entity>(alloc, 1, transform.GetPosition(), transform.GetRotation(), (mesh), *material);
		entities.push_back(machine);
		break;
	}
}

void RacingScene::Start()
{
}
