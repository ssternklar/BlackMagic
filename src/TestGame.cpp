#include "TestGame.h"
#include "DX11Renderer.h"

using namespace BlackMagic;
using namespace DirectX;

TestGame::TestGame(BlackMagic::PlatformBase* platformBase)
	: GameAbstraction(platformBase),
	_camera({ 0, 0, -10 }, { 0, 0, 0, 1 })
{
	XMFLOAT4 d;
	XMStoreFloat4(&d, XMQuaternionIdentity());
}

void TestGame::Destroy()
{
	for(auto o : _objects)
	{
		allocator.deallocate(o, true);
	}
}

void TestGame::Init(BlackMagic::byte* gameMemory, size_t memorySize)
{
	allocator = BestFitAllocator(32, memorySize, gameMemory);
	LoadContent();

	unsigned int width, height;
	platform->GetScreenDimensions(&width, &height);
	_camera.UpdateProjectionMatrix(width, height);

	_globalLight = {
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 1, -1, 1 },
		0,
		{ 0, 1, 1 }
	};
}

void TestGame::LoadContent()
{
	auto content = platform->GetContentManager();
	XMFLOAT4 quatIdentity;
	XMStoreFloat4(&quatIdentity, XMQuaternionIdentity());
	XMFLOAT3 defaultScale = { 1, 1, 1 };

	auto sphere = content->Load<Mesh>(L"/models/sphere.obj");
	auto gPassVS = content->Load<VertexShader>(L"/shaders/GBufferVS.cso");
	auto gPassPS = content->Load<PixelShader>(L"/shaders/GBufferPS.cso");
	auto sphereTex = content->Load<Texture>(L"/textures/test_texture.png");
	auto sphereNormals = content->Load<Texture>(L"/textures/test_normals.png");

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	auto sampler = reinterpret_cast<DX11Renderer*>(platform->GetRenderer())->CreateSamplerState(samplerDesc);
	auto mat = std::make_shared<Material> (
		gPassVS, gPassPS,
		sphereTex, sampler,
		sphereNormals
	);

	for(float y = 0; y < 11; y++)
	{
		for (float x = 0; x < 11; x++)
		{
			auto mem = allocator.allocate<Entity>();
			_objects.push_back(new (mem) Entity(XMFLOAT3{ x, y, 0 }, XMFLOAT4{ 0, 0, 0, 1 }, sphere, mat));
		}
	}
}

void TestGame::Update(float deltaTime)
{
	if (platform->GetInputData()->GetButton(15))
	{
		shouldExit = true;
	}
	_camera.Update(deltaTime);

	TransformData::GetSingleton()->UpdateTransforms();
}

void TestGame::Draw(float deltaTime)
{
	auto renderer = platform->GetRenderer();
	const XMFLOAT4 color{ 0.4f, 0.6f, 0.75f, 0.0f };
	renderer->Clear(color);
	std::vector<Entity*> renderables;

	renderables.reserve(121);
	renderer->Cull(_camera, _objects, renderables);
	renderer->Render(_camera, renderables, _globalLight);
	renderer->Present(0, 0);
}