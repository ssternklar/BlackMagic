#include "Scene.h"
#include "Assets.h"
#include "FileFormats.h"
#include "FileUtil.h"

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

	h = ProxyHandler::Get();
	h->willExport = false;

	AssetManager::Instance().TrackAsset<SceneData>(h, fullPath);
	sceneExportConfig.push_back(h);
	activeScene = h;

	Save(h);

	return h;
}

SceneData::Handle SceneData::Get(std::string name)
{
	std::string fullPath = root + name + ".scene";

	Handle h = AssetManager::Instance().GetHandle<SceneData>(fullPath);
	if (h.ptr())
		return h;

	h = Load(fullPath);

	if (!h.ptr())
		return h;

	AssetManager::Instance().TrackAsset<SceneData>(h, fullPath);

	return h;
}

SceneData::Handle SceneData::Load(std::string scenePath)
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

	uint8_t willExport;
	size_t numEntities, meshIndex;
	fread_s(&willExport, sizeof(uint8_t), sizeof(willExport), 1, sceneFile);
	fread_s(&numEntities, sizeof(size_t), sizeof(numEntities), 1, sceneFile);
	
	h->willExport = willExport == 1;
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

void SceneData::Save(Handle handle)
{
	Asset<SceneData> asset = AssetManager::Instance().GetAsset<SceneData>(handle);

	FILE* sceneFile;
	fopen_s(&sceneFile, asset.path.c_str(), "wb");
	if (!sceneFile)
	{
		printf("failed to save scene '%s'\n", asset.path.c_str());
		return;
	}

	uint8_t willExport = (handle->willExport ? 1 : 0);
	size_t numEntities, meshIndex;
	numEntities = handle->entities.size();

	fwrite(&willExport, sizeof(uint8_t), 1, sceneFile);
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

void SceneData::Export(std::string path, Handle handle)
{
	FileUtil::CreateDirectoryRecursive(path);

	FILE* sceneFile;
	fopen_s(&sceneFile, path.c_str(), "wb");
	if (!sceneFile)
	{
		printf("Failed to write scene file '%s'", path.c_str());
		return;
	}

	Export::Scene::File fileData = {};
	fileData.numEntities = (uint16_t)handle->entities.size();

	vector<uint16_t> uniqueMeshUIDs;
	vector<uint16_t> meshUIDs;
	uint16_t meshUID;
	size_t i;
	for (i = 0; i < fileData.numEntities; ++i)
	{
		meshUID = (uint16_t)AssetManager::Instance().GetAsset<MeshData>(handle->entities[i]->mesh).uID;

		meshUIDs.push_back(meshUID);

		auto check = std::find(uniqueMeshUIDs.begin(), uniqueMeshUIDs.end(), meshUID);
		if (check == uniqueMeshUIDs.end())
			uniqueMeshUIDs.push_back(meshUID);
	}

	fileData.numAssets = (uint16_t)uniqueMeshUIDs.size();
	fwrite(&fileData.numAssets, sizeof(Export::Scene::File::numAssets), 1, sceneFile);

	if (fileData.numAssets > 0)
		fwrite(&uniqueMeshUIDs[0], sizeof(Export::Scene::Entity::meshUID), uniqueMeshUIDs.size(), sceneFile);

	fwrite(&fileData.numEntities, sizeof(Export::Scene::File::numEntities), 1, sceneFile);

	Export::Scene::Transform transform = {};
	EntityData::Handle entity;
	for (i = 0; i < fileData.numEntities; ++i)
	{
		entity = handle->entities[i];

		memcpy_s(&transform.pos[0], sizeof(float) * 3, &entity->transform->pos.x, sizeof(DirectX::XMFLOAT3));
		memcpy_s(&transform.rot[0], sizeof(float) * 4, &entity->transform->rot.x, sizeof(DirectX::XMFLOAT4));
		memcpy_s(&transform.scale, sizeof(float), &entity->transform->scale, sizeof(float));

		fwrite(&transform.pos[0], sizeof(Export::Scene::Transform), 1, sceneFile);
		fwrite(&meshUIDs[i], sizeof(Export::Scene::Entity::meshUID), 1, sceneFile);
	}

	fclose(sceneFile);
}
