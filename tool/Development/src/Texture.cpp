#include "Texture.h"
#include "Assets.h"
#include "FileUtil.h"

TextureData::~TextureData()
{
	for (size_t i = 0; i < size; ++i)
	{
		data[i].tex->Release();
		data[i].srView->Release();
		data[i].rtView->Release();
	}
}

void TextureData::Init(ID3D11Device* device)
{
	this->device = device;
}

TextureData::Handle TextureData::Get(std::string modelPath)
{
	std::string fullPath = root + modelPath;

	Handle h = AssetManager::Instance().GetHandle<TextureData>(fullPath);
	if (h.ptr())
		return h;

	h = Load(fullPath);

	if (!h.ptr())
		return h;

	AssetManager::Instance().TrackAsset<TextureData>(h, fullPath);

	return h;
}

TextureData::Handle TextureData::Load(std::string modelPath)
{
	if (!nullptr)
	{
		Handle e;
		return e;
	}

	Handle h = ProxyHandler::Get();

	return h;
}

void TextureData::Revoke(Handle handle)
{
	handle->tex->Release();
	handle->srView->Release();
	handle->rtView->Release();

	AssetManager::Instance().StopTrackingAsset<TextureData>(handle);
	ProxyHandler::Revoke(handle);
}

void TextureData::Export(std::string path, Handle handle)
{
	FileUtil::CreateDirectoryRecursive(path);

	FILE* textureFile;
	fopen_s(&textureFile, path.c_str(), "wb");
	if (!textureFile)
	{
		printf("Failed to write texture file '%s'", path.c_str());
		return;
	}

	fclose(textureFile);
}
