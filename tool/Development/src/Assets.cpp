#include "Assets.h"

AssetManager::AssetManager()
{
	ready = false;
}

bool AssetManager::IsReady()
{
	return ready;
}

void AssetManager::CreateProject(std::string folder)
{
	// create folders
	// write files
	LoadProject(folder);
}

void AssetManager::LoadProject(std::string folder)
{
	// read manifest

	// set defaults
	MeshData::Handle h = MeshData::Instance().GetDirect("assets/defaults/teapot.obj");
	SetDefault<MeshData>(h);
	TrackAsset<MeshData>(h, "assets/defaults/teapot.obj");
	GetAsset<MeshData>(0).name = "default";

	// load remaining files

	// scenes?...

	ready = true;
}