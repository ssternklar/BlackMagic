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

	fclose(projFile);
	
	FileUtil::WriteResourceToDisk(IDR_MESH1, "mesh", "assets/defaults/defaultMesh.obj");

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

	string defaultMeshPath = FileUtil::GetStringInFile(projFile);

	fclose(projFile);

	MeshData::Handle h = MeshData::Instance().GetDirect(defaultMeshPath);
	SetDefault<MeshData>(h);
	TrackAsset<MeshData>(h, defaultMeshPath);
	GetAsset<MeshData>(0).name = "default"; // TODO won't work with mid-usage loading

	// load all files in

	// scene...stuff.....hotswappable scenes?

	ready = true;

	return true;
}

void AssetManager::SaveProject()
{

}
