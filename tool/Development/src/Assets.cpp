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
	FileUtil::CreateDirectoryRecursive("assets/shaders/");
	FileUtil::CreateDirectoryRecursive("assets/scenes/");

	FILE* projFile;
	fopen_s(&projFile, "churo.proj", "wb");
	if (!projFile)
	{
		printf("failed to create project: could not create churo.proj\n");
		return false;
	}

	const char* defaultMeshPath = "assets/defaults/defaultMesh.obj";
	fwrite(defaultMeshPath, strlen(defaultMeshPath) + 1, 1, projFile);
	size_t zero = 0;
	fwrite(&zero, sizeof(size_t), 1, projFile);

	fclose(projFile);
	
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
	string defaultMeshPath = FileUtil::GetStringInFile(projFile);
	MeshData::Handle h = MeshData::Instance().GetDirect(defaultMeshPath);
	SetDefault<MeshData>(h);
	TrackAsset<MeshData>(h, defaultMeshPath).name = "default";

	// load mesh assets
	size_t meshCount;
	string meshPath;
	fread_s(&meshCount, sizeof(size_t), sizeof(size_t), 1, projFile);
	for (size_t i = 0; i < meshCount; ++i)
	{
		meshPath = FileUtil::GetStringInFile(projFile);
		h = MeshData::Instance().GetDirect(meshPath);
		TrackAsset<MeshData>(h, meshPath).name = FileUtil::GetStringInFile(projFile);
	}

	fclose(projFile);

	// scene...stuff.....hotswappable scenes?

	ready = true;

	return true;
}

void AssetManager::SaveProject()
{
	Tracker<MeshData>& meshTracker = trackers;
	MeshData::Handle& defaultMesh = defaults;
	Asset<MeshData>& defaultMeshAsset = GetAsset<MeshData>(defaultMesh);

	FILE* projFile;
	fopen_s(&projFile, "churo.proj", "wb");
	if (!projFile)
	{
		printf("failed to save project: could not write to churo.proj\n");
		return;
	}

	// save defaults
	fwrite(defaultMeshAsset.path.c_str(), defaultMeshAsset.path.length() + 1, 1, projFile);
	
	// save metadata
	size_t meshCount = meshTracker.assets.size() - 1;
	fwrite(&meshCount, sizeof(size_t), 1, projFile);

	// save mesh assets
	for (size_t i = 0; i < meshTracker.assets.size(); ++i)
	{
		if (meshTracker.assets[i].handle != defaultMesh)
		{
			fwrite(meshTracker.assets[i].path.c_str(), meshTracker.assets[i].path.length() + 1, 1, projFile);
			fwrite(meshTracker.assets[i].name.c_str(), meshTracker.assets[i].name.length() + 1, 1, projFile);
		}
	}

	fclose(projFile);
}
