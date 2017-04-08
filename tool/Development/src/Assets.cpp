#include "Assets.h"
#include "FileUtil.h"
#include "../resource.h"
#include "FileFormats.h"

AssetManager::AssetManager()
{
	ready = false;
}

bool AssetManager::IsReady()
{
	return ready;
}

bool AssetManager::CreateProject(std::string folder)
{
	if (!FileUtil::IsFolderEmpty(folder))
		return false;

	SetCurrentDirectoryA(folder.c_str());

	FileUtil::CreateDirectoryRecursive("assets/defaults/");
	FileUtil::CreateDirectoryRecursive("assets/models/");
	FileUtil::CreateDirectoryRecursive("assets/scenes/");

	FILE* projFile;
	fopen_s(&projFile, "churo.proj", "wb");
	if (!projFile)
	{
		printf("failed to create project: could not create churo.proj\n");
		return false;
	}

	// metadata
	Internal::Proj::Meta meta = {};
	fwrite(&meta.nextUID, sizeof(Internal::Proj::Meta), 1, projFile);

	// default assets
	const char* defaultName = "default";
	const char* defaultMeshPath = "assets/defaults/Mesh.obj";

	fwrite(&meta.defaultMeshUID, sizeof(Internal::Proj::Meta::defaultMeshUID), 1, projFile);
	fwrite(defaultMeshPath, strlen(defaultMeshPath) + 1, 1, projFile);
	fwrite(defaultName, strlen(defaultName) + 1, 1, projFile);

	// camera
	float origin[7] = {};
	origin[6] = 1.0f;
	fwrite(&origin, sizeof(float), 7, projFile);

	fclose(projFile);
	
	// write default files to disk
	FileUtil::WriteResourceToDisk(IDR_MESH1, "mesh", defaultMeshPath);

	LoadProject(folder);

	return true;
}

// TODO if i add loading mid-usage of the tool, track which assets are not used and revoke them
bool AssetManager::LoadProject(std::string folder)
{
	if (FileUtil::IsFolderEmpty(folder))
		return false;

	SetCurrentDirectoryA(folder.c_str());

	FILE* projFile;
	fopen_s(&projFile, "churo.proj", "rb");
	if (!projFile)
	{
		printf("failed to load project: could not open churo.proj\n");
		return false;
	}

	// load metadata
	Internal::Proj::Meta meta;
	fread_s(&meta.nextUID, sizeof(Internal::Proj::Meta), sizeof(Internal::Proj::Meta), 1, projFile);

	// load assets
	Asset<MeshData> meshAsset = {};
	for (size_t i = 0; i < meta.numMeshes; ++i)
	{
		fread_s(&meshAsset.uID, sizeof(Internal::Proj::Asset::uID), sizeof(Internal::Proj::Asset::uID), 1, projFile);
		meshAsset.path = FileUtil::GetStringInFile(projFile);
		meshAsset.name = FileUtil::GetStringInFile(projFile);
		meshAsset.handle = MeshData::Instance().LoadMesh(meshAsset.path);
		AddAsset(meshAsset);

		if (meshAsset.uID == meta.defaultMeshUID)
			SetDefault<MeshData>(meshAsset.handle);
	}

	Asset<SceneData> sceneAsset;
	for (size_t i = 0; i < meta.numScenes; ++i)
	{
		fread_s(&sceneAsset.uID, sizeof(Internal::Proj::Asset::uID), sizeof(Internal::Proj::Asset::uID), 1, projFile);
		sceneAsset.path = FileUtil::GetStringInFile(projFile);
		sceneAsset.name = FileUtil::GetStringInFile(projFile);
		sceneAsset.handle = SceneData::Instance().LoadScene(sceneAsset.path);
		AddAsset(sceneAsset);
	}

	// load camera
	fread_s(&Camera::Instance().transform->pos, sizeof(DirectX::XMFLOAT3), sizeof(float), 3, projFile);
	fread_s(&Camera::Instance().transform->rot, sizeof(DirectX::XMFLOAT4), sizeof(float), 4, projFile);

	fclose(projFile);

	ready = true;

	return true;
}

void AssetManager::SaveProject()
{
	FILE* projFile;
	fopen_s(&projFile, "churo.proj", "wb");
	if (!projFile)
	{
		printf("failed to save project: could not write to churo.proj\n");
		return;
	}

	// gather up the trackers and defaults
	Tracker<MeshData>& meshTracker = trackers;
	Tracker<SceneData>& sceneTracker = trackers;

	MeshData::Handle& defaultMesh = defaults;
	Asset<MeshData>& defaultMeshAsset = GetAsset<MeshData>(defaultMesh);
	
	// save metadata
	Internal::Proj::Meta meta = {
		nextUID,
		defaultMeshAsset.uID,
		meshTracker.assets.size(),
		sceneTracker.assets.size()
	};
	fwrite(&meta.nextUID, sizeof(Internal::Proj::Meta), 1, projFile);

	// save assets
	for (size_t i = 0; i < meta.numMeshes; ++i)
	{
		fwrite(&meshTracker.assets[i].uID, sizeof(Internal::Proj::Asset::uID), 1, projFile);
		fwrite(meshTracker.assets[i].path.c_str(), meshTracker.assets[i].path.length() + 1, 1, projFile);
		fwrite(meshTracker.assets[i].name.c_str(), meshTracker.assets[i].name.length() + 1, 1, projFile);
	}

	for (size_t i = 0; i < meta.numScenes; ++i)
	{
		SceneData::Instance().SaveScene(sceneTracker.assets[i].handle);
		fwrite(&sceneTracker.assets[i].uID, sizeof(Internal::Proj::Asset::uID), 1, projFile);
		fwrite(sceneTracker.assets[i].path.c_str(), sceneTracker.assets[i].path.length() + 1, 1, projFile);
		fwrite(sceneTracker.assets[i].name.c_str(), sceneTracker.assets[i].name.length() + 1, 1, projFile);
	}

	// save camera
	fwrite(&Camera::Instance().transform->pos, sizeof(float), 3, projFile);
	fwrite(&Camera::Instance().transform->rot, sizeof(float), 4, projFile);

	fclose(projFile);
}

bool AssetManager::Export(std::string name, bool force)
{
	string exportFolder = "exports/" + name + "/";
	FileUtil::CreateDirectoryRecursive(exportFolder);

	if (!force && !FileUtil::IsFolderEmpty(exportFolder))
		return false;

	if (force)
		FileUtil::DeleteDirectory(exportFolder);

	Tracker<SceneData>& sceneTracker = trackers;
	
	// rewrite the export for new UID format

	return true;
}
