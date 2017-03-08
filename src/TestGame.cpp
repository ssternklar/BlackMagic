#include "TestGame.h"
#include "DX11Renderer.h"
#include "TestJob.h";

using namespace BlackMagic;
using namespace ECS;
using namespace DirectX;

TestGame::TestGame(BlackMagic::PlatformBase* platformBase)
	: GameAbstraction(platformBase)
{}

void TestGame::Destroy()
{
	_gameWorld->destroyWorld();
}

void TestGame::Init(BlackMagic::byte* gameMemory, size_t memorySize)
{
	allocator = BestFitAllocator(32, memorySize, gameMemory);
	//auto adapter = AllocatorSTLAdapter<Entity, BestFitAllocator>(allocator);
	//gameWorld = ECS::World::createWorld(ECS::Allocator(adapter));
	_gameWorld = ECS::World::createWorld(AllocatorSTLAdapter<ECS::Entity, BestFitAllocator>(&allocator));
	LoadContent();

	unsigned int width, height;
	Entity* cam = _gameWorld->create();
	XMFLOAT4 quatIdentity;
	XMStoreFloat4(&quatIdentity, XMQuaternionIdentity());

	cam->assign<Transform>(XMFLOAT3{ 0,0,0 }, quatIdentity, XMFLOAT3{ 1,1,1 });
	_camera = cam->assign<Camera>(XMFLOAT3{ 0,0,0 });
	platform->GetScreenDimensions(&width, &height);
	_camera->UpdateProjectionMatrix(width, height);

	_globalLight = {
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0, -1, 1 },
		0,
		{ 0, 1, 1 }
	};
}

void TestGame::LoadContent()
{
	auto content = platform->GetContentManager();
}

void TestGame::Update(float deltaTime)
{
	if (platform->GetInputData()->GetButton(15))
	{
		shouldExit = true;
	}
	// World update
	// this ticks all registered systems
	_gameWorld->tick(deltaTime);
	auto cam = _gameWorld->each<Transform, Camera>().begin().get();
	_camera->Update(&cam->get<Transform>().get());

	TransformData::GetSingleton()->UpdateTransforms();
	auto job = platform->GetThreadManager()->CreateGenericJob<TestJob>();
	job->WaitUntilJobIsComplete();
	platform->GetThreadManager()->DestroyGenericJob(job);
}

void TestGame::Draw(float deltaTime)
{
	auto renderer = platform->GetRenderer();
	const XMFLOAT4 color{ 0.4f, 0.6f, 0.75f, 0.0f };
	renderer->Clear(color);
	std::vector<Entity*> renderables;

	renderables.reserve(100);
	renderer->Cull(_camera.get(), _gameWorld, renderables);
	renderer->Render(_camera.get(), renderables, _globalLight);
	renderer->Present(0, 0);
}