#include "Assets.h"
#include "FileUtil.h"
#include "../resource.h"

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

	// default paths
	const char* defaultMeshPath = "assets/defaults/Mesh.obj";
	fwrite(defaultMeshPath, strlen(defaultMeshPath) + 1, 1, projFile);

	// metadata
	size_t zero[2] = {};
	fwrite(&zero, sizeof(size_t), 2, projFile);

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

bool AssetManager::LoadProject(std::string folder)
{
	if (FileUtil::IsFolderEmpty(folder))
		return false;

	// TODO
	// if i add loading mid-usage of the tool, track which assets are not used and revoke them

	SetCurrentDirectoryA(folder.c_str());

	FILE* projFile;
	fopen_s(&projFile, "churo.proj", "rb");
	if (!projFile)
	{
		printf("failed to load project: could not open churo.proj\n");
		return false;
	}

	// load defaults
	string path = FileUtil::GetStringInFile(projFile);
	MeshData::Handle mesh = MeshData::Instance().LoadMesh(path);
	SetDefault<MeshData>(mesh);
	TrackAsset<MeshData>(mesh, path).name = "default";

	// load metadata
	size_t meshCount, sceneCount;
	fread_s(&meshCount, sizeof(size_t), sizeof(size_t), 1, projFile);
	fread_s(&sceneCount, sizeof(size_t), sizeof(size_t), 1, projFile);

	// load assets
	for (size_t i = 0; i < meshCount; ++i)
	{
		path = FileUtil::GetStringInFile(projFile);
		mesh = MeshData::Instance().LoadMesh(path);
		TrackAsset<MeshData>(mesh, path).name = FileUtil::GetStringInFile(projFile);
	}

	SceneData::Handle scene;
	for (size_t i = 0; i < sceneCount; ++i)
	{
		path = FileUtil::GetStringInFile(projFile);
		scene = SceneData::Instance().LoadScene(path);
		TrackAsset<SceneData>(scene, path).name = FileUtil::GetStringInFile(projFile);
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

	// save defaults
	fwrite(defaultMeshAsset.path.c_str(), defaultMeshAsset.path.length() + 1, 1, projFile);
	
	// save metadata
	size_t meshCount = meshTracker.assets.size() - 1;
	fwrite(&meshCount, sizeof(size_t), 1, projFile);

	size_t sceneCount = sceneTracker.assets.size();
	fwrite(&sceneCount, sizeof(size_t), 1, projFile);

	// save assets
	for (size_t i = 0; i < meshTracker.assets.size(); ++i)
	{
		if (meshTracker.assets[i].handle != defaultMesh)
		{
			fwrite(meshTracker.assets[i].path.c_str(), meshTracker.assets[i].path.length() + 1, 1, projFile);
			fwrite(meshTracker.assets[i].name.c_str(), meshTracker.assets[i].name.length() + 1, 1, projFile);
		}
	}

	for (size_t i = 0; i < sceneCount; ++i)
	{
		SceneData::Instance().SaveScene(sceneTracker.assets[i].handle);
		fwrite(sceneTracker.assets[i].path.c_str(), sceneTracker.assets[i].path.length() + 1, 1, projFile);
		fwrite(sceneTracker.assets[i].name.c_str(), sceneTracker.assets[i].name.length() + 1, 1, projFile);
	}

	// save camera
	fwrite(&Camera::Instance().transform->pos, sizeof(float), 3, projFile);
	fwrite(&Camera::Instance().transform->rot, sizeof(float), 4, projFile);

	fclose(projFile);
}
