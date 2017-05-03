#include "Scene.h"
#include "PlatformBase.h"
#include "ContentManager.h"
using namespace BlackMagic;

BlackMagic::Scene::Scene(BlackMagic::BestFitAllocator* allocator)
{
	alloc = allocator;
}

void Scene::Init(AssetPointer<SceneDesc> desc)
{
	auto file = (uint16_t*)(desc->fileHandle);
	auto uidCount = *file;
	auto uids = ++file;
	file += uidCount;
	auto sceneFileEntityCount = *file;
	file++;

	AssetPointer<UnknownContentType>* b = alloc->allocate<BlackMagic::AssetPointer<UnknownContentType>>(uidCount);
	ContentManager* cm = PlatformBase::GetSingleton()->GetContentManager();
	for (int i = 0; i < uidCount; i++)
	{
		b[i] = cm->Load<UnknownContentType>(uids[i]);
	}

	sceneAssets = b;
	sceneAssetCount = uidCount;

	byte* entities = (byte*)(file);

	for (int i = 0; i < sceneFileEntityCount; i++)
	{
		float* tStuff = (float*)entities;
		entities += sizeof(float) * 12;
		Transform t(CreateVector3(tStuff[0], tStuff[1], tStuff[2]), CreateQuaternion(tStuff[4], tStuff[5], tStuff[6], tStuff[7]), CreateVector3(tStuff[8], tStuff[8], tStuff[8]));
		uint16_t tag = *(uint16_t*)(entities);
		entities += sizeof(uint16_t);
		AssetPointer<Mesh> mesh = cm->Load<Mesh>(*(uint16_t*)(entities));
		entities += sizeof(uint16_t);
		AssetPointer<Material> mat = cm->Load<Material>(*(uint16_t*)(entities));
		ProcessType(tag, t, mesh, mat);
	}
}

Scene::~Scene()
{
	if(sceneAssetCount > 0)
	{
		DestructAndDeallocate<AssetPointer_Base>(alloc, sceneAssets, sceneAssetCount);
	}
	PlatformBase::GetSingleton()->GetContentManager()->AssetGC();
}
