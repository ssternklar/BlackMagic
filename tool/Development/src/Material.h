#pragma once

#include <string>

#include "Patterns.h"
#include "Shader.h"

struct Material
{
	VertexShaderData::Handle vertexShader;
	PixelShaderData::Handle pixelShader;
};

class MaterialData : public ProxyHandler<Material, MaterialData>
{
public:
	Handle Create(std::string name);
	Handle Get(std::string materialPath);
	void Revoke(Handle handle);

	Handle Load(std::string path);
	void Save(Handle handle);
	void Export(std::string path, Handle handle);
	const std::string root = "assets/materials/";
};
