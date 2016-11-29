#include "FZERO.h"
#include "DirectXGraphicsDevice.h"
using namespace BlackMagic;
using namespace ECS;
using namespace DirectX;

FZERO::~FZERO()
{
	platform->GetGraphicsDevice()->CleanupBuffer(splineMesh->VertexBuffer());
	platform->GetGraphicsDevice()->CleanupBuffer(splineMesh->IndexBuffer());
	gameWorld->unregisterSystem(sys);
	gameWorld->destroyWorld();
}

void FZERO::Init(BlackMagic::byte* gameMemory, size_t memorySize)
{

	allocator = BestFitAllocator(32, memorySize, gameMemory);
	//auto adapter = AllocatorSTLAdapter<Entity, BestFitAllocator>(allocator);
	//gameWorld = ECS::World::createWorld(ECS::Allocator(adapter));
	gameWorld = ECS::World::createWorld();
	LoadContent();
	DirectionalLight light = {
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 1, 0, 0 }
	};
	_directionalLights.push_back(light);
	DirectionalLight light2 = {
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ -1, 0, 0 }
	};
	_directionalLights.push_back(light2);
}

void FZERO::LoadContent()
{
	auto _content = platform->GetContentManager();
	splineMesh = std::make_shared<Mesh>();
	_spline = _content->Load<Spline>(L"spline.bin");
	_spline->GenerateMesh(platform->GetGraphicsDevice(), splineMesh.get());

	auto sphere = _content->Load<Mesh>(L"/models/sphere.obj");

	auto rocks = _content->Load<Texture>(L"/textures/rock.jpg");
	auto rocksNormals = _content->Load<Texture>(L"/textures/rockNormals.jpg");

	auto sand = _content->Load<Texture>(L"/textures/sand_texture.JPG");
	auto sandNormals = _content->Load<Texture>(L"textures/sand_normal.JPG");

	auto gPassVS = _content->Load<VertexShader>(L"/shaders/GBufferVS.cso");
	auto gPassPS = _content->Load<PixelShader>(L"/shaders/GBufferPS.cso");

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	auto sampler = ((DirectXGraphicsDevice*)platform->GetGraphicsDevice())->CreateSamplerState(samplerDesc);

	auto gridMat = std::make_shared<Material>(
		gPassVS, gPassPS,
		rocks, sampler,
		rocksNormals);

	XMFLOAT4 quatIdentity;
	DirectX::XMStoreFloat4(&quatIdentity, DirectX::XMQuaternionIdentity());
	XMFLOAT3 defaultScale = { 1, 1, 1 };

	// Add our test system

	sys = allocator.allocate<MachineSystem>();
	sys = new (sys) MachineSystem(_spline);
	gameWorld->registerSystem(sys);

	Entity* ent = gameWorld->create();
	ent->assign<Transform>(XMFLOAT3{ 0,0,0 }, quatIdentity, defaultScale);
	ent->assign<Renderable>(splineMesh, gridMat);

	Entity* machine = gameWorld->create();
	machine->assign<Transform>(XMFLOAT3{ 0,0,0 }, quatIdentity, defaultScale);
	machine->assign<Renderable>(sphere, gridMat);
	machine->assign<Machine>();
	_camera = machine->assign<Camera>(XMFLOAT3{ 0, 1, -1 });

	unsigned int width, height;
	platform->GetScreenDimensions(&width, &height);
	_camera->UpdateProjectionMatrix(width, height);
}

void FZERO::Update(float deltaTime)
{
	if (platform->GetInputData()->GetButton(15))
	{
		//Quit()
	}
	// World update
	// this ticks all registered systems
	gameWorld->tick(deltaTime);

	TransformData::GetSingleton()->UpdateTransforms();
}

void FZERO::Draw(float deltaTime)
{
	auto _renderer = platform->GetGraphicsDevice();
	const XMFLOAT4 color{ 0.4f, 0.6f, 0.75f, 0.0f };
	_renderer->Clear(color);
	std::vector<Entity*> renderables;
	renderables.reserve(100);
	_renderer->Cull(_camera.get(), gameWorld, renderables);
	_renderer->Render(_camera.get(), renderables, _directionalLights);
	_renderer->Present(0, 0);
}