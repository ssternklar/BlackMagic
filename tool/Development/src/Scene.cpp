#include "Scene.h"
#include "Assets.h"

void SceneData::Init(GuiData::EntityEditorData* gui)
{
	this->gui = gui;
}

SceneData::Handle SceneData::Create(std::string name)
{
	std::string fullPath = root + name + ".scene";

	Handle h = AssetManager::Instance().GetHandle<SceneData>(fullPath);
	if (h.ptr())
		return h;

	FILE* sceneFile;
	fopen_s(&sceneFile, fullPath.c_str(), "wb");
	if (!sceneFile)
	{
		Handle e;
		return e;
	}

	size_t numEntities = 0;
	fwrite(&numEntities, sizeof(size_t), 1, sceneFile);

	fclose(sceneFile);

	h = ProxyHandler::Get();

	AssetManager::Instance().TrackAsset<SceneData>(h, fullPath);
	activeScene = h;

	return h;
}

SceneData::Handle SceneData::Get(std::string name)
{
	std::string fullPath = root + name + ".scene";

	Handle h = AssetManager::Instance().GetHandle<SceneData>(fullPath);
	if (h.ptr())
		return h;

	h = LoadScene(fullPath);

	if (!h.ptr())
		return h;

	AssetManager::Instance().TrackAsset<SceneData>(h, fullPath);

	return h;
}

SceneData::Handle SceneData::LoadScene(std::string scenePath)
{
	FILE* sceneFile;
	fopen_s(&sceneFile, scenePath.c_str(), "rb");
	if (!sceneFile)
	{
		Handle e;
		return e;
	}

	Handle h = ProxyHandler::Get();
	EntityData::Handle entity;

	size_t numEntities, meshIndex;
	fread_s(&numEntities, sizeof(size_t), sizeof(numEntities), 1, sceneFile);
	h->entities.reserve(numEntities);

	for (size_t i = 0; i < numEntities; ++i)
	{
		entity = EntityData::Instance().Get();
		h->entities.push_back(entity);
		fread_s(&entity->transform->pos, sizeof(DirectX::XMFLOAT3), sizeof(float), 3, sceneFile);
		fread_s(&entity->transform->scale, sizeof(float), sizeof(float), 1, sceneFile);
		fread_s(&entity->transform->rot, sizeof(DirectX::XMFLOAT4), sizeof(float), 4, sceneFile);

		fread_s(&meshIndex, sizeof(size_t), sizeof(size_t), 1, sceneFile);
		entity->mesh = AssetManager::Instance().GetAsset<MeshData>(meshIndex).handle;
	}

	fclose(sceneFile);

	return h;
}

void SceneData::SaveScene(Handle handle)
{
	Asset<SceneData> asset = AssetManager::Instance().GetAsset<SceneData>(handle);

	FILE* sceneFile;
	fopen_s(&sceneFile, asset.path.c_str(), "wb");
	if (!sceneFile)
	{
		printf("failed to save scene '%s'\n", asset.path.c_str());
		return;
	}

	size_t numEntities, meshIndex;
	numEntities = handle->entities.size();
	fwrite(&numEntities, sizeof(size_t), 1, sceneFile);

	for (size_t i = 0; i < numEntities; ++i)
	{
		fwrite(&handle->entities[i]->transform->pos, sizeof(float), 3, sceneFile);
		fwrite(&handle->entities[i]->transform->scale, sizeof(float), 1, sceneFile);
		fwrite(&handle->entities[i]->transform->rot, sizeof(float), 4, sceneFile);

		meshIndex = AssetManager::Instance().GetIndex<MeshData>(handle->entities[i]->mesh);
		fwrite(&meshIndex, sizeof(size_t), 1, sceneFile);
	}

	fclose(sceneFile);
}

SceneData::Handle SceneData::CurrentScene()
{
	return activeScene;
}

void SceneData::SwapScene(size_t index)
{
	if (index >= size)
		return;

	activeScene = AssetManager::Instance().GetAsset<SceneData>(index).handle;
	SelectEntity(activeScene->selectedEntity);
}

void SceneData::SelectEntity(EntityData::Handle entity)
{
	if (!activeScene.ptr())
		return;

	if (entity.ptr() && std::find(activeScene->entities.begin(), activeScene->entities.end(), entity) != activeScene->entities.end())
	{
		gui->meshIndex = (int)AssetManager::Instance().GetIndex<MeshData>(entity->mesh);
		activeScene->selectedEntity = entity;
	}
	else
	{
		EntityData::Handle e;
		activeScene->selectedEntity = e;
		gui->meshIndex = -1;
	}
}

void SceneData::Revoke(Handle handle)
{
	AssetManager::Instance().StopTrackingAsset<SceneData>(handle);
	ProxyHandler::Revoke(handle);
}
