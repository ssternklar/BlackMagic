#include "RacingScene.h"
#include "SceneBasedGame.h"
#include "ContentManager.h"
#include "RacingStartJob.h"
using namespace BlackMagic;

template<> SceneBasedGame<RacingScene>* SceneBasedGame<RacingScene>::singleton = nullptr;

RacingScene::RacingScene(BlackMagic::BestFitAllocator* allocator) :
	BlackMagic::Scene(allocator),
	entities(AllocatorAdapter<Entity*>(allocator))
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
	Transform cameraTransform;
	if(gameIsStarted)
	{
		for (Entity* entity : entities)
		{
			entity->Update(deltaTime);
		}

		if (machine)
		{
			float distSquared;
			float mt = spline->GuessNearestPoint(machine->GetTransform().GetPosition(), &distSquared);
			if (mt > halfPoint && lastT < halfPoint)
			{
				crossedHalf = true;
			}
			if (mt > start && lastT < start && crossedHalf)
			{
				if(lap < 3)
				{
					lights[lap]->_material = *lightMats[2];
					PlatformBase::GetSingleton()->GetAudioManager()->PlayOneShot(startBoops[1].get(), 1);
					lap++;
				}
				else
				{
					PlatformBase::GetSingleton()->GetAudioManager()->PlayOneShot(startBoops[1].get(), 1);
					PlatformBase::GetSingleton()->GetAudioManager()->StopBGM();
					SceneBasedGame<RacingScene>::GetSingleton()->StartSceneLoad("scenes/title.scene");
				}
			}
			lastT = mt;
		}
	}

	if (machine)
	{
		cameraTransform = machine->GetTransform();
	}

	camera->Update(cameraTransform);
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
		machine = AllocateAndConstruct<Machine>(alloc, 1, transform.GetPosition(), transform.GetRotation(), (mesh), *material);
		entities.push_back(machine);
		break;
	case SceneTags::LIGHT0:
	case SceneTags::LIGHT1:
	case SceneTags::LIGHT2:
		lights[tag - (int)SceneTags::LIGHT0] = AllocateAndConstruct<Entity>(alloc, 1, transform.GetPosition(), transform.GetRotation(), (mesh), *material);
		lightMats[tag - (int)SceneTags::LIGHT0] = material;
		entities.push_back(lights[tag - (int)SceneTags::LIGHT0]);
		break;
	}
}

void RacingScene::Start()
{
	PlatformBase* platform = PlatformBase::GetSingleton();
	BlackMagic::ContentJob<WAVFile>* boop1Job = platform->GetThreadManager()->CreateContentJob<WAVFile>("misc/boop1.wav");
	BlackMagic::ContentJob<WAVFile>* boop2Job = platform->GetThreadManager()->CreateContentJob<WAVFile>("misc/boop2.wav");
	if (machine)
	{
		BlackMagic::ContentJob<WAVFile>* bgmJob = platform->GetThreadManager()->CreateContentJob<WAVFile>("misc/bgm.wav");
		bgm = bgmJob->GetResult();
		platform->GetAudioManager()->PlayBGM(bgm.get(), .5f);
		platform->GetThreadManager()->DestroyContentJob<WAVFile>(bgmJob);
	}
	startBoops[0] = boop1Job->GetResult();
	startBoops[1] = boop2Job->GetResult();
	platform->GetThreadManager()->DestroyContentJob<WAVFile>(boop1Job);
	platform->GetThreadManager()->DestroyContentJob<WAVFile>(boop2Job);

	//Machine presence is signal for menu vs game
	if (machine)
	{
		machine->Init(spline);
		machine->Update(0);
		platform->GetThreadManager()->CreateGenericJob<RacingStartJob>(lightMats, lights, startBoops, &gameIsStarted);
		float distSquared;
		start = spline->GuessNearestPoint(machine->GetTransform().GetPosition(), &distSquared);
		halfPoint = start + .5;
		halfPoint -= (int)halfPoint;
		lastT = start;
	}
	else
	{
		cursor->Init(startBoops);
		gameIsStarted = true;
	}
}
