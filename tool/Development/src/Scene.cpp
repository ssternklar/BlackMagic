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
	size_t numEntities, numMisc, meshIndex, materialIndex, miscIndex;
	fread_s(&willExport, sizeof(uint8_t), sizeof(willExport), 1, sceneFile);
	fread_s(&numEntities, sizeof(size_t), sizeof(numEntities), 1, sceneFile);
	fread_s(&numMisc, sizeof(size_t), sizeof(numMisc), 1, sceneFile);
	
	h->willExport = willExport == 1;
	h->entities.reserve(numEntities);
	h->misc.reserve(numMisc);
	
	for (size_t i = 0; i < numEntities; ++i)
	{
		entity = EntityData::Instance().Get();
		h->entities.push_back(entity);

		fread_s(&entity->type, sizeof(Internal::Scene::Entity::type), sizeof(Internal::Scene::Entity::type), 1, sceneFile);
		fread_s(&entity->transform->pos, sizeof(DirectX::XMFLOAT3), sizeof(float), 3, sceneFile);
		fread_s(&entity->transform->scale, sizeof(float), sizeof(float), 1, sceneFile);
		fread_s(&entity->transform->rot, sizeof(DirectX::XMFLOAT4), sizeof(float), 4, sceneFile);

		fread_s(&meshIndex, sizeof(size_t), sizeof(size_t), 1, sceneFile);
		entity->mesh = AssetManager::Instance().GetAsset<MeshData>(meshIndex).handle;

		fread_s(&materialIndex, sizeof(size_t), sizeof(size_t), 1, sceneFile);
		entity->material = AssetManager::Instance().GetAsset<MaterialData>(materialIndex).handle;
	}

	for (size_t i = 0; i < numMisc; ++i)
	{
		fread_s(&miscIndex, sizeof(size_t), sizeof(size_t), 1, sceneFile);
		h->misc.push_back(AssetManager::Instance().GetAsset<MiscData>(miscIndex).handle);
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
	size_t numEntities, numMisc, meshIndex, materialIndex, miscIndex;
	numEntities = handle->entities.size();
	numMisc = handle->misc.size();

	fwrite(&willExport, sizeof(uint8_t), 1, sceneFile);
	fwrite(&numEntities, sizeof(size_t), 1, sceneFile);
	fwrite(&numMisc, sizeof(size_t), 1, sceneFile);

	for (size_t i = 0; i < numEntities; ++i)
	{
		fwrite(&handle->entities[i]->type, sizeof(Internal::Scene::Entity::type), 1, sceneFile);
		fwrite(&handle->entities[i]->transform->pos, sizeof(float), 3, sceneFile);
		fwrite(&handle->entities[i]->transform->scale, sizeof(float), 1, sceneFile);
		fwrite(&handle->entities[i]->transform->rot, sizeof(float), 4, sceneFile);

		meshIndex = AssetManager::Instance().GetIndex<MeshData>(handle->entities[i]->mesh);
		fwrite(&meshIndex, sizeof(size_t), 1, sceneFile);

		materialIndex = AssetManager::Instance().GetIndex<MaterialData>(handle->entities[i]->material);
		fwrite(&materialIndex, sizeof(size_t), 1, sceneFile);
	}

	for (size_t i = 0; i < numMisc; ++i)
	{
		miscIndex = AssetManager::Instance().GetIndex<MiscData>(handle->misc[i]);
		fwrite(&miscIndex, sizeof(size_t), 1, sceneFile);
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
		gui->materialIndex = (int)AssetManager::Instance().GetIndex<MaterialData>(entity->material);
		activeScene->selectedEntity = entity;
	}
	else
	{
		EntityData::Handle e;
		activeScene->selectedEntity = e;
		gui->meshIndex = -1;
		gui->materialIndex = -1;
	}
}

void SceneData::Revoke(Handle handle)
{
	AssetManager::Instance().StopTrackingAsset<SceneData>(handle);
	ProxyHandler::Revoke(handle);
}

std::vector<uint16_t> SceneData::Export(std::string path, Handle handle)
{
	FileUtil::CreateDirectoryRecursive(path);

	vector<uint16_t> uIDs;

	FILE* sceneFile;
	fopen_s(&sceneFile, path.c_str(), "wb");
	if (!sceneFile)
	{
		printf("Failed to write scene file '%s'", path.c_str());
		return uIDs;
	}

	Export::Scene::File fileData = {};
	fileData.numEntities = (uint16_t)handle->entities.size();

	uint16_t meshUID, matUID;
	size_t i;
	for (i = 0; i < fileData.numEntities; ++i)
	{
		meshUID = (uint16_t)AssetManager::Instance().GetAsset<MeshData>(handle->entities[i]->mesh).uID;
		matUID = (uint16_t)AssetManager::Instance().GetAsset<MaterialData>(handle->entities[i]->material).uID;

		auto check = std::find(uIDs.begin(), uIDs.end(), meshUID);
		if (check == uIDs.end())
			uIDs.push_back(meshUID);

		check = std::find(uIDs.begin(), uIDs.end(), matUID);
		if (check == uIDs.end())
			uIDs.push_back(matUID);
	}

	for (i = 0; i < handle->misc.size(); ++i)
		uIDs.push_back((uint16_t)AssetManager::Instance().GetAsset<MiscData>(handle->misc[i]).uID);

	fileData.numAssets = (uint16_t)uIDs.size();
	fwrite(&fileData.numAssets, sizeof(Export::Scene::File::numAssets), 1, sceneFile);

	if (fileData.numAssets > 0)
		fwrite(&uIDs[0], sizeof(uint16_t), uIDs.size(), sceneFile);

	fwrite(&fileData.numEntities, sizeof(Export::Scene::File::numEntities), 1, sceneFile);

	Export::Scene::Transform transform = {};
	Export::Scene::Entity entityData = {};
	EntityData::Handle entity;
	for (i = 0; i < fileData.numEntities; ++i)
	{
		entity = handle->entities[i];

		memcpy_s(&transform.pos[0], sizeof(float) * 3, &entity->transform->pos.x, sizeof(DirectX::XMFLOAT3));
		memcpy_s(&transform.rot[0], sizeof(float) * 4, &entity->transform->rot.x, sizeof(DirectX::XMFLOAT4));
		memcpy_s(&transform.scale, sizeof(float), &entity->transform->scale, sizeof(float));

		fwrite(&transform.pos[0], sizeof(Export::Scene::Transform), 1, sceneFile);

		entityData.type = entity->type;
		entityData.meshUID = (uint16_t)AssetManager::Instance().GetAsset<MeshData>(entity->mesh).uID;
		entityData.materialUID = (uint16_t)AssetManager::Instance().GetAsset<MaterialData>(entity->material).uID;

		fwrite(&entityData.type, sizeof(uint16_t), 3, sceneFile);
	}

	fclose(sceneFile);

	return uIDs;
}
