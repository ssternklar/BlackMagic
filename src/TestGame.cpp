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
	auto sphereNormals = content->Load<Texture>(L"/textures/blank_normals.png");

	auto sampler = platform->GetRenderer()->CreateSampler();
	auto mat = Material(
		allocator,
		gPassVS, gPassPS
	);

	mat.SetResource("normalMap", Material::ResourceStage::PS, sphereNormals, Material::ResourceStorageType::Static);
	mat.SetResource("mainSampler", Material::ResourceStage::PS, sampler, Material::ResourceStorageType::Static);
	
	for(float y = 0; y < 11; y++)
	{
		unsigned int metalness = 255;
		XMFLOAT4 albedo = { 0, 0, 0, 1.0f };
		TextureDesc desc;
		desc = { 0 };

		switch (static_cast<unsigned int>(y))
		{
			case 0:
				albedo = { 0.56f, 0.57f, 0.58f, 1.0f };
				break;
			case 1:
				albedo = { 0.972f, 0.960f, 0.915f, 1.0f };
				break;
			case 2:
				albedo = { 0.913f, 0.921f, 0.925f, 1.0f };
				break;
			case 3:
				albedo = { 1.000f, 0.766f, 0.336f, 1.0f };
				break;
			case 4:
				albedo = { 0.955f, 0.637f, 0.538f, 1.0f };
				break;
			case 5:
				albedo = { 0.550f, 0.556f, 0.554f, 1.0f };
				break;
			case 6:
				albedo = { 0.660f, 0.609f, 0.526f, 1.0f };
				break;
			case 7:
				albedo = { 0.542f, 0.497f, 0.449f, 1.0f };
				break;
			case 8:
				albedo = { 0.662f, 0.655f, 0.634f, 1.0f };
				break;
			case 9:
				albedo = { 0.672f, 0.637f, 0.585f, 1.0f };
				break;
			case 10:
				albedo = { 0.02f, 0.02f, 0.02f, 1.0f };
				metalness = 0;
				break;
			case 11:
				albedo = { 0.81f, 0.81f, 0.81f, 1.0f };
				metalness = 0;
				break;
		}

		desc.Format = Texture::Format::R8G8B8A8_UNORM;
		desc.Width = 1;
		desc.Height = 1;
		desc.GPUUsage = Texture::Usage::READ;
		desc.InitialData = &albedo;
		desc.Type = Texture::Type::FLAT_2D;
		auto albedoTex = std::make_shared<Texture>(platform->GetRenderer()->CreateTexture(desc));

		desc.Format = Texture::Format::R8_UNORM;
		desc.InitialData = &metalness;
		auto metalnessTex = std::make_shared<Texture>(platform->GetRenderer()->CreateTexture(desc));

		auto rowMaterial = mat;
		rowMaterial.SetResource("albedoMap", Material::ResourceStage::PS, albedoTex, Material::ResourceStorageType::Static);
		rowMaterial.SetResource("metalnessMap", Material::ResourceStage::PS, metalnessTex, Material::ResourceStorageType::Static);

		for (float x = 0; x < 11; x++)
		{
			unsigned int roughness = (x / 10.0f) * 255;
			desc.InitialData = &roughness;
			desc.Format = Texture::Format::R8_UNORM;
			auto roughnessTex = std::make_shared<Texture>(platform->GetRenderer()->CreateTexture(desc));

			auto mem = allocator.allocate<Entity>();
			auto matInstance = rowMaterial;
			matInstance.SetResource("roughnessMap", Material::ResourceStage::PS, roughnessTex, Material::ResourceStorageType::Instance);
			_objects.push_back(new (mem) Entity(XMFLOAT3{ x, y, 0 }, XMFLOAT4{ 0, 0, 0, 1 }, sphere, matInstance));
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