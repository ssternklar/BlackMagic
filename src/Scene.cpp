#include "Scene.h"
#include "PlatformBase.h"
#include "ContentManager.h"
using namespace BlackMagic;

void Scene::Init(BlackMagic::BestFitAllocator* sceneAllocator, AssetPointer<SceneDesc> desc)
{
	auto file = (uint16_t*)(desc->fileHandle);
	auto uidCount = *file;
	auto uids = ++file;
	uids += uidCount;
	auto sceneFileEntityCount = *file;
	byte* entities = (byte*)(++file);

	alloc = sceneAllocator;
	AssetPointer<UnknownContentType>* b = alloc->allocate<BlackMagic::AssetPointer<UnknownContentType>>(uidCount);
	ContentManager* cm = PlatformBase::GetSingleton()->GetContentManager();
	for (int i = 0; i < uidCount; i++)
	{
		b[i] = cm->Load<UnknownContentType>(uids[i]);
	}

	sceneAssets = b;
	sceneAssetCount = uidCount;

	for (int i = 0; i < sceneFileEntityCount; i++)
	{
		BlackMagic::byte* current = entities + (sceneFileEntityCount + SIZE_OF_SCENE_FILE_ENTITY);
		Transform* tptr = reinterpret_cast<Transform*>(current);
		Transform t = *tptr;
		current += SIZE_OF_TRANSFORM_FLOATS;
		uint16_t tag = *(uint16_t*)(current);
		current += sizeof(uint16_t);
		AssetPointer<Mesh> mesh = cm->Load<Mesh>(*(uint16_t*)(current));
		current += sizeof(uint16_t);
		AssetPointer<Material> mat = cm->Load<Material>(*(uint16_t*)(current));
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
