#include "Material.h"
#include "Assets.h"
#include "FileUtil.h"
#include "FileFormats.h"

MaterialData::~MaterialData()
{
	sampler->Release();
}

void MaterialData::Init(ID3D11Device* device)
{
	this->device = device;

	D3D11_SAMPLER_DESC desc = {};
	desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	desc.Filter = D3D11_FILTER_ANISOTROPIC;
	desc.MaxAnisotropy = D3D11_DEFAULT_MAX_ANISOTROPY;
	desc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&desc, &sampler);
}

MaterialData::Handle MaterialData::Create(std::string name)
{
	std::string fullPath = root + name + ".mat";

	Handle h = AssetManager::Instance().GetHandle<MaterialData>(fullPath);
	if (h.ptr())
		return h;

	h = ProxyHandler::Get();
	
	h->vertexShader = AssetManager::Instance().defaults;
	FlushPixelShader(h, AssetManager::Instance().defaults);

	AssetManager::Instance().TrackAsset<MaterialData>(h, fullPath);

	Save(h);

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

MaterialData::Handle MaterialData::Load(std::string materialPath)
{
	FILE* materialFile;
	fopen_s(&materialFile, materialPath.c_str(), "rb");
	if (!materialFile)
	{
		Handle e;
		return e;
	}

	Handle h = ProxyHandler::Get();
	Internal::Material::File fileData = {};

	fread_s(&fileData.vertexShaderIndex, sizeof(size_t) * 3, sizeof(size_t), 3, materialFile);

	h->vertexShader = AssetManager::Instance().GetAsset<VertexShaderData>(fileData.vertexShaderIndex).handle;
	FlushPixelShader(h, AssetManager::Instance().GetAsset<PixelShaderData>(fileData.PixelShaderIndex).handle);

	size_t textureIndex;
	for (size_t i = 0; i < fileData.numTextures; ++i)
	{
		fread_s(&textureIndex, sizeof(size_t), sizeof(size_t), 1, materialFile);
		h->textures[i] = AssetManager::Instance().GetAsset<TextureData>(textureIndex).handle;
	}

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

	Internal::Material::File fileData = {};

	fileData.vertexShaderIndex = AssetManager::Instance().GetIndex<VertexShaderData>(handle->vertexShader);
	fileData.PixelShaderIndex = AssetManager::Instance().GetIndex<PixelShaderData>(handle->pixelShader);
	fileData.numTextures = handle->textures.size();

	fwrite(&fileData.vertexShaderIndex, sizeof(size_t), 3, materialFile);

	size_t textureIndex;
	for (size_t i = 0; i < fileData.numTextures; ++i)
	{
		textureIndex = AssetManager::Instance().GetIndex<TextureData>(handle->textures[i]);
		fwrite(&textureIndex, sizeof(size_t), 1, materialFile);
	}

	fclose(materialFile);
}

void MaterialData::Revoke(Handle handle)
{
	AssetManager::Instance().StopTrackingAsset<MaterialData>(handle);
	ProxyHandler::Revoke(handle);
}

std::vector<uint16_t> MaterialData::Export(std::string path, Handle handle)
{
	FileUtil::CreateDirectoryRecursive(path);

	std::vector<uint16_t> uIDs;

	FILE* materialFile;
	fopen_s(&materialFile, path.c_str(), "wb");
	if (!materialFile)
	{
		printf("Failed to write material file '%s'", path.c_str());
		return uIDs;
	}

	Export::Material::File fileData = {};

	fileData.shaders.vertexShaderUID = (uint16_t)AssetManager::Instance().GetAsset<VertexShaderData>(handle->vertexShader).uID;
	fileData.shaders.pixelShaderUID = (uint16_t)AssetManager::Instance().GetAsset<PixelShaderData>(handle->pixelShader).uID;
	fileData.numTextures = (uint8_t)handle->pixelShader->textures.size();
	fileData.numSamplers = (uint8_t)handle->pixelShader->samplers.size();

	uIDs.reserve(2 + fileData.numTextures);

	fwrite(&fileData.shaders.vertexShaderUID, sizeof(uint16_t), 3, materialFile);

	std::vector<uint16_t> nameIndices;
	nameIndices.reserve(fileData.numTextures + fileData.numSamplers);
	std::string nameBlob;
	size_t i;

	for (i = 0; i < fileData.numTextures; ++i)
	{
		nameIndices.push_back((uint16_t)nameBlob.size());
		nameBlob += handle->pixelShader->textures[i] + '\0';
	}

	for (i = 0; i < fileData.numSamplers; ++i)
	{
		nameIndices.push_back((uint16_t)nameBlob.size());
		nameBlob += handle->pixelShader->samplers[i] + '\0';
	}

	uint16_t textureUID;
	for (i = 0; i < fileData.numTextures; ++i)
	{
		fwrite(&nameIndices[i], sizeof(uint16_t), 1, materialFile);

		textureUID = (uint16_t)AssetManager::Instance().GetAsset<TextureData>(handle->textures[i]).uID;
		uIDs.push_back(textureUID);
		fwrite(&textureUID, sizeof(uint16_t), 1, materialFile);
	}

	if (fileData.numSamplers > 0)
		fwrite(&nameIndices[fileData.numTextures], sizeof(uint16_t), fileData.numSamplers, materialFile);
	if (fileData.numTextures + fileData.numSamplers > 0)
		fwrite(nameBlob.c_str(), sizeof(char), nameBlob.size(), materialFile);

	fclose(materialFile);

	uIDs.push_back(fileData.shaders.vertexShaderUID);
	uIDs.push_back(fileData.shaders.pixelShaderUID);
	return uIDs;
}

void MaterialData::Use(Handle handle)
{
	handle->vertexShader->shader->SetShader();
	handle->pixelShader->shader->SetShader();

	std::string name;
	size_t i;

	for (i = 0; i < handle->textures.size(); ++i)
	{
		name = handle->pixelShader->textures[i];
		handle->pixelShader->shader->SetShaderResourceView(name, handle->textures[i]->srv);
	}

	for (i = 0; i < handle->pixelShader->samplers.size(); ++i)
	{
		name = handle->pixelShader->samplers[i];
		handle->pixelShader->shader->SetSamplerState(name, sampler);
	}

	handle->vertexShader->shader->CopyAllBufferData();
	handle->pixelShader->shader->CopyAllBufferData();
}

bool MaterialData::FlushPixelShader(Handle handle, PixelShaderData::Handle newPixelShader)
{
	if (newPixelShader == handle->pixelShader)
		return false;

	handle->pixelShader = newPixelShader;
	handle->textures.clear();

	for (size_t i = 0; i < newPixelShader->textures.size(); ++i)
		handle->textures.push_back(AssetManager::Instance().defaults);

	return true;
}
