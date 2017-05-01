#include "TestGame.h"
#include "Renderer.h"
#include "TestJob.h"
#include "BMMath.h"
#include "WAVFile.h"
#include "ContentManager.h"
#include "PlatformBase.h"
using namespace BlackMagic;

TestGame::TestGame() :
	_camera({ 0, 0, -10 }, { 0, 0, 0, 1 })
{
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
	PlatformBase::GetSingleton()->GetScreenDimensions(&width, &height);
	_camera.UpdateProjectionMatrix(width, height);

	_globalLight = {
		CreateVector4(0.0f, 0.0f, 0.0f, 1.0f),
		CreateVector4(10.0f, 10.0f, 10.0f, 1.0f),
		CreateVector3(1, -1, 1),
		CreateVector3(0, 1, 1)
	};
}

void TestGame::LoadContent()
{
#ifdef BM_PLATFORM_WINDOWS	
	auto content = PlatformBase::GetSingleton()->GetContentManager();
	Quaternion quatIdentity = CreateQuaternionIdentity();
	Vector3 defaultScale = CreateVector3(1.0f, 1.0f, 1.0f);
	auto sphere = std::shared_ptr<Mesh>(content->UntrackedLoad<Mesh>("/models/sphere.bmmesh"));
	auto plane = std::shared_ptr<Mesh>(content->UntrackedLoad<Mesh>("/models/plane.bmmesh"));
	auto gPassVS = content->Load<VertexShader>(std::string("/shaders/GBufferVS.cso"));
	auto gPassPS = content->Load<PixelShader>(std::string("/shaders/GBufferPS.cso"));
	auto sphereTex = content->Load<Texture>(std::string("/textures/test_texture.png"));
	auto sphereNormals = content->Load<Texture>(std::string("/textures/blank_normals.png"));
	auto sampler = PlatformBase::GetSingleton()->GetRenderer()->CreateSampler();
	auto mat = Material(
		allocator,
		gPassVS, gPassPS
	);

	std::shared_ptr<Texture> planeAlbedo;
	std::shared_ptr<Texture> planeMetal;
	std::shared_ptr<Texture> planeRoughness;

	mat.SetResource("normalMap", Material::ResourceStage::PS, sphereNormals, Material::ResourceStorageType::Static);
	mat.SetResource("mainSampler", Material::ResourceStage::PS, sampler, Material::ResourceStorageType::Static);
	

	for(float y = 0; y < 12; y++)
	{
		unsigned int metalness = 255;
		Vector4 albedo = CreateVector4(0.0f, 0.0f, 0.0f, 1.0f);
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
		albedo = albedo * 255.0f;
		albedo = Floor(albedo);
		unsigned char c[4] = { static_cast<unsigned char>(albedo.data[0]), 
			static_cast<unsigned char>(albedo.data[1]), 
			static_cast<unsigned char>(albedo.data[2]), 
			static_cast<unsigned char>(albedo.data[3])
		};


		desc.Format = Texture::Format::R8G8B8A8_UNORM;
		desc.Width = 1;
		desc.Height = 1;
		desc.GPUUsage = Texture::Usage::READ;
		desc.InitialData = c;
		desc.Type = Texture::Type::FLAT_2D;
		auto albedoTex = std::make_shared<Texture>(PlatformBase::GetSingleton()->GetRenderer()->CreateTexture(desc));

		desc.Format = Texture::Format::R8_UNORM;
		desc.InitialData = &metalness;
		auto metalnessTex = std::make_shared<Texture>(PlatformBase::GetSingleton()->GetRenderer()->CreateTexture(desc));

		auto rowMaterial = mat;
		rowMaterial.SetResource("albedoMap", Material::ResourceStage::PS, albedoTex, Material::ResourceStorageType::Static);
		rowMaterial.SetResource("metalnessMap", Material::ResourceStage::PS, metalnessTex, Material::ResourceStorageType::Static);

		for (float x = 1; x < 12; x++)
		{
			unsigned int roughness = static_cast<unsigned int>(((x-1.0f)/ 11.0f) * 255);

			desc.InitialData = &roughness;
			desc.Format = Texture::Format::R8_UNORM;
			auto roughnessTex = std::make_shared<Texture>(PlatformBase::GetSingleton()->GetRenderer()->CreateTexture(desc));

			auto mem = allocator.allocate<Entity>();
			auto matInstance = rowMaterial;
			matInstance.SetResource("roughnessMap", Material::ResourceStage::PS, roughnessTex, Material::ResourceStorageType::Instance);
			auto p = CreateVector3(x, y, 0);
			_objects.push_back(new (mem) Entity(CreateVector3(x, y, 0), CreateQuaternionIdentity(), sphere, matInstance));
		
			if (static_cast<unsigned int>(y) == 11 && static_cast<unsigned int>(x) == 1)
			{
				planeAlbedo = albedoTex;
				planeRoughness = roughnessTex;
				planeMetal = metalnessTex;
			}
		}
	}

	auto planeMem = allocator.allocate<Entity>();
	mat.SetResource("albedoMap", Material::ResourceStage::PS, planeAlbedo, Material::ResourceStorageType::Static);
	mat.SetResource("metalnessMap", Material::ResourceStage::PS, planeMetal, Material::ResourceStorageType::Static);
	mat.SetResource("roughnessMap", Material::ResourceStage::PS, planeRoughness, Material::ResourceStorageType::Instance);
	auto ptr = new (planeMem) Entity(CreateVector3(5.5f, 5.5f, 2.0f), CreateQuaternion(CreateVector3(1.0, 0, 0), -3.14f / 2.0f), plane, mat);
	ptr->GetTransform().SetScale(CreateVector3(11.0f, 1.0f, 11.0f));
	_objects.push_back(ptr);
	//_objects.push_back(new (planeMem) Entity(CreateVector3(0, 0, 2.0f), CreateQuaternion(CreateVector3(1.0, 0, 0), -3.14f/2.0f), plane, mat));
#endif
	auto sdr = PlatformBase::GetSingleton()->GetContentManager()->UntrackedLoad<WAVFile>("CityEscape.wav");
	//PlatformBase::GetSingleton()->GetAudioManager()->PlayBGM(sdr, .6f);
}

void TestGame::Update(float deltaTime)
{
	if (PlatformBase::GetSingleton()->GetInputData()->GetButton(15))
	{
		shouldExit = true;
	}
	_camera.Update(deltaTime);
	printf("%f\n", deltaTime);
	/*auto job = platform->GetThreadManager()->CreateGenericJob<TestJob>();
	job->WaitUntilJobIsComplete();
	platform->GetThreadManager()->DestroyGenericJob(job);*/
}

void TestGame::Draw(float deltaTime)
{
	auto renderer = PlatformBase::GetSingleton()->GetRenderer();
	const Vector4 color{ 0.4f, 0.6f, 0.75f, 0.0f };
	renderer->Clear(color);
	std::vector<Entity*> renderables;

	renderables.reserve(121);
	renderer->Cull(_camera, _objects, renderables);
	renderer->Render(_camera, renderables, _globalLight);
	renderer->Present(0, 0);
}