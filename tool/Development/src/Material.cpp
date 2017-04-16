#include "Material.h"
#include "Assets.h"
#include "FileUtil.h"
#include "FileFormats.h"

MaterialData::Handle MaterialData::Create(std::string name)
{
	std::string fullPath = root + name + ".mat";

	Handle h = AssetManager::Instance().GetHandle<MaterialData>(fullPath);
	if (h.ptr())
		return h;

	FILE* materialFile;
	fopen_s(&materialFile, fullPath.c_str(), "wb");
	if (!materialFile)
	{
		Handle e;
		return e;
	}

	h = ProxyHandler::Get();
	h->vertexShader = AssetManager::Instance().defaults;
	h->pixelShader = AssetManager::Instance().defaults;

	// write material file contents from handle

	fclose(materialFile);

	AssetManager::Instance().TrackAsset<MaterialData>(h, fullPath);

	return h;
}

MaterialData::Handle MaterialData::Get(std::string name)
{
	std::string fullPath = root + name + ".mat";

	Handle h = AssetManager::Instance().GetHandle<MaterialData>(fullPath);
	if (h.ptr())
		return h;

	h = Load(fullPath);

	if (!h.ptr())
		return h;

	AssetManager::Instance().TrackAsset<MaterialData>(h, fullPath);

	return h;
}

MaterialData::Handle MaterialData::Load(std::string scenePath)
{
	FILE* materialFile;
	fopen_s(&materialFile, scenePath.c_str(), "rb");
	if (!materialFile)
	{
		Handle e;
		return e;
	}

	Handle h = ProxyHandler::Get();

	// read material and populate handle

	fclose(materialFile);

	return h;
}

void MaterialData::Save(Handle handle)
{
	Asset<MaterialData> asset = AssetManager::Instance().GetAsset<MaterialData>(handle);

	FILE* materialFile;
	fopen_s(&materialFile, asset.path.c_str(), "wb");
	if (!materialFile)
	{
		printf("failed to save material '%s'\n", asset.path.c_str());
		return;
	}

	// save material from handle

	fclose(materialFile);
}

void MaterialData::Revoke(Handle handle)
{
	AssetManager::Instance().StopTrackingAsset<MaterialData>(handle);
	ProxyHandler::Revoke(handle);
}

void MaterialData::Export(std::string path, Handle handle)
{
	FileUtil::CreateDirectoryRecursive(path);

	FILE* materialFile;
	fopen_s(&materialFile, path.c_str(), "wb");
	if (!materialFile)
	{
		printf("Failed to write material file '%s'", path.c_str());
		return;
	}

	// export material from handle

	fclose(materialFile);
}