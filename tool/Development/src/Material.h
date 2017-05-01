#pragma once

#include <vector>
#include <string>

#include "Patterns.h"
#include "Shader.h"
#include "Texture.h"

struct Material
{
	VertexShaderData::Handle vertexShader;
	PixelShaderData::Handle pixelShader;

	std::vector<TextureData::Handle> textures;
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
	std::vector<uint16_t> Export(std::string path, Handle handle);
	const std::string root = "assets/materials/";

	void Use(Handle handle);
	bool FlushPixelShader(Handle handle, PixelShaderData::Handle newPixelShader);

private:
	ID3D11Device* device;
	ID3D11SamplerState* sampler;
};
