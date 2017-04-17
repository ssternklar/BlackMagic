#pragma once

#include <string>
#include <unordered_map>

#include "Patterns.h"
#include "Shader.h"
#include "Texture.h"

struct Material
{
	VertexShaderData::Handle vertexShader;
	PixelShaderData::Handle pixelShader;

	struct Resource
	{
		enum Stage : unsigned char
		{
			VS = 1 << 0,
			PS = 1 << 4
		};

		enum class Type : unsigned char
		{
			Data,
			Texture,
			Sampler
		};

		Stage stage;
		Type type;
		size_t size;
		void* data;

		~Resource()
		{
			if (type == Type::Sampler)
				static_cast<ID3D11SamplerState*>(data)->Release();
			if (type == Type::Data)
				delete data;
		}
	};

	std::unordered_map<std::string, Resource*> resources;
};

class MaterialData : public ProxyHandler<Material, MaterialData>
{
public:
	~MaterialData();

	void Init(ID3D11Device* device);

	Handle Create(std::string name);
	Handle Get(std::string materialPath);
	void Revoke(Handle handle);

	Handle Load(std::string path);
	void Save(Handle handle);
	void Export(std::string path, Handle handle);
	const std::string root = "assets/materials/";

	void Use(Handle handle);
	void SetResource(Handle handle, std::string name, Material::Resource::Stage s, size_t size, void* data);
	void SetResource(Handle handle, std::string name, Material::Resource::Stage s, TextureData::Handle texture);
	void SetResource(Handle handle, std::string name, Material::Resource::Stage s, ID3D11SamplerState* sampler);

private:
	ID3D11Device* device;
	ID3D11SamplerState* sampler;
	void UploadData(std::string name, const Material::Resource* resource, Handle handle);
};
