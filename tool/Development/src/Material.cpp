#include "Material.h"
#include "Assets.h"
#include "FileUtil.h"
#include "FileFormats.h"

MaterialData::~MaterialData()
{
	for (size_t i = 0; i < size; ++i)
		for (auto& pair : data[i].resources)
			delete pair.second;

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
	h->pixelShader = AssetManager::Instance().defaults;

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

	Internal::Mat::File fileData = {};

	fread_s(&fileData.shaders.vertexShaderIndex, sizeof(Internal::Mat::Shaders), sizeof(Internal::Mat::Shaders), 1, materialFile);
	fread_s(&fileData.numTextures, sizeof(uint8_t), sizeof(uint8_t), 3, materialFile);

	h->vertexShader = AssetManager::Instance().GetAsset<VertexShaderData>(fileData.shaders.vertexShaderIndex).handle;
	h->pixelShader = AssetManager::Instance().GetAsset<PixelShaderData>(fileData.shaders.pixelShaderIndex).handle;

	using MR = Material::Resource;

	size_t i;
	MR::Stage resourceStage;
	size_t meta;
	std::string resourceName;

	for (i = 0; i < fileData.numTextures; ++i)
	{
		fread_s(&resourceStage, sizeof(MR::Stage), sizeof(MR::Stage), 1, materialFile);
		fread_s(&meta, sizeof(Internal::Mat::TextureResource::index), sizeof(Internal::Mat::TextureResource::index), 1, materialFile);
		resourceName = FileUtil::GetStringInFile(materialFile);

		SetResource(h, resourceName, resourceStage, AssetManager::Instance().GetAsset<TextureData>(meta).handle);
	}

	uint8_t* data;

	for (i = 0; i < fileData.numDatas; ++i)
	{
		fread_s(&resourceStage, sizeof(MR::Stage), sizeof(MR::Stage), 1, materialFile);
		fread_s(&meta, sizeof(Internal::Mat::DataResource::sizeInBytes), sizeof(Internal::Mat::DataResource::sizeInBytes), 1, materialFile);

		if (meta > 0)
			data = new uint8_t[meta];
		fread_s(data, meta, 1, meta, materialFile);

		resourceName = FileUtil::GetStringInFile(materialFile);

		SetResource(h, resourceName, resourceStage, meta, data);

		if (meta)
			delete data;
	}

	for (i = 0; i < fileData.numSamplers; ++i)
	{
		fread_s(&resourceStage, sizeof(MR::Stage), sizeof(MR::Stage), 1, materialFile);
		resourceName = FileUtil::GetStringInFile(materialFile);

		SetResource(h, resourceName, resourceStage, sampler);
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

	Internal::Mat::File fileData = {};

	fileData.shaders.vertexShaderIndex = AssetManager::Instance().GetIndex<VertexShaderData>(handle->vertexShader);
	fileData.shaders.pixelShaderIndex = AssetManager::Instance().GetIndex<PixelShaderData>(handle->pixelShader);

	fwrite(&fileData.shaders.vertexShaderIndex, sizeof(Internal::Mat::Shaders), 1, materialFile);

	using MR = Material::Resource;

	vector<std::pair<std::string, MR*>> textures;
	vector<std::pair<std::string, MR*>> datas;
	vector<std::pair<std::string, MR*>> samplers;

	for (auto& pair : handle->resources)
	{
		switch (pair.second->type)
		{
		case MR::Type::Texture:
			textures.push_back(pair);
			break;
		case MR::Type::Data:
			datas.push_back(pair);
			break;
		case MR::Type::Sampler:
			samplers.push_back(pair);
			break;
		}
	}

	fileData.numTextures = (uint8_t)textures.size();
	fileData.numDatas = (uint8_t)datas.size();
	fileData.numSamplers = (uint8_t)samplers.size();

	fwrite(&fileData.numTextures, sizeof(uint8_t), 3, materialFile);

	size_t i;
	size_t textureIndex;
	std::string resourceName;

	for (i = 0; i < fileData.numTextures; ++i)
	{
		fwrite(&textures[i].second->stage, sizeof(MR::Stage), 1, materialFile);
		
		textureIndex = AssetManager::Instance().GetIndex<TextureData>((*static_cast<TextureData::Handle*>(textures[i].second->data)));
		fwrite(&textureIndex, sizeof(Internal::Mat::TextureResource::index), 1, materialFile);
		
		resourceName = textures[i].first + '\0';
		fwrite(resourceName.c_str(), resourceName.length(), 1, materialFile);
	}

	for (i = 0; i < fileData.numDatas; ++i)
	{
		fwrite(&datas[i].second->stage, sizeof(MR::Stage), 1, materialFile);

		fwrite(&datas[i].second->size, sizeof(Internal::Mat::DataResource::sizeInBytes), 1, materialFile);
		fwrite(datas[i].second->data, sizeof(uint8_t), datas[i].second->size, materialFile);

		resourceName = datas[i].first + '\0';
		fwrite(resourceName.c_str(), resourceName.length(), 1, materialFile);
	}

	for (i = 0; i < fileData.numSamplers; ++i)
	{
		fwrite(&samplers[i].second->stage, sizeof(MR::Stage), 1, materialFile);

		resourceName = samplers[i].first + '\0';
		fwrite(resourceName.c_str(), resourceName.length(), 1, materialFile);
	}

	fclose(materialFile);
}

void MaterialData::Revoke(Handle handle)
{
	for (auto& pair : handle->resources)
		delete pair.second;

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

void MaterialData::Use(Handle handle)
{
	(*handle->vertexShader)->SetShader();
	(*handle->pixelShader)->SetShader();

	for (auto& pair : handle->resources)
		UploadData(pair.first, pair.second, handle);

	(*handle->vertexShader)->CopyAllBufferData();
	(*handle->pixelShader)->CopyAllBufferData();
}

void MaterialData::SetResource(Handle handle, std::string name, Material::Resource::Stage s, size_t size, void* data)
{
	Material::Resource* dat = new Material::Resource;
	dat->stage = s;
	dat->size = size;
	dat->type = Material::Resource::Type::Data;
	dat->data = new unsigned char[dat->size];
	memcpy(dat->data, data, dat->size);

	handle->resources[name] = dat;
}

void MaterialData::SetResource(Handle handle, std::string name, Material::Resource::Stage s, TextureData::Handle texture)
{
	Material::Resource* dat = new Material::Resource;
	dat->stage = s;
	dat->size = sizeof(texture);
	dat->type = Material::Resource::Type::Texture;
	dat->data = new TextureData::Handle;
	memcpy(dat->data, &texture, dat->size);

	handle->resources[name] = dat;
}

void MaterialData::SetResource(Handle handle, std::string name, Material::Resource::Stage s, ID3D11SamplerState* sampler)
{
	Material::Resource* dat = new Material::Resource;
	dat->stage = s;
	dat->size = sizeof(sampler);
	dat->type = Material::Resource::Type::Sampler;
	dat->data = sampler;
	sampler->AddRef();

	handle->resources[name] = dat;
}

void MaterialData::UploadData(std::string name, const Material::Resource* resource, Handle handle)
{
	using MR = Material::Resource;

	MR::Stage s = resource->stage;

	switch (resource->type)
	{
	case MR::Type::Data:
		if (s & MR::Stage::VS)
			(*handle->vertexShader)->SetData(name, resource->data, resource->size);
		if (s & MR::Stage::PS)
			(*handle->pixelShader)->SetData(name, resource->data, resource->size);
		break;
	case MR::Type::Sampler:
	{
		ID3D11SamplerState* sampler = static_cast<ID3D11SamplerState*>(resource->data);
		if (s & MR::Stage::VS)
			(*handle->vertexShader)->SetSamplerState(name, sampler);
		if (s & MR::Stage::PS)
			(*handle->pixelShader)->SetSamplerState(name, sampler);
		break;
	}
	case MR::Type::Texture:
	{
		ID3D11ShaderResourceView* srv = (*static_cast<TextureData::Handle*>(resource->data))->srv;
		if (s & MR::Stage::VS)
			(*handle->vertexShader)->SetShaderResourceView(name, srv);
		if (s & MR::Stage::PS)
			(*handle->pixelShader)->SetShaderResourceView(name, srv);
		break;
	}
	}
}