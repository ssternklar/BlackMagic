#pragma once

#include <vector>
#include <string>

#include "Patterns.h"
#include "SimpleShader.h"

template <class T>
struct Shader
{
	T* shader;

	std::vector<std::string> samplers;
	std::vector<std::string> textures;
};

template <class T>
class ShaderData : public ProxyHandler<Shader<T>, ShaderData<T>>
{
public:
	~ShaderData();

	void Init(ID3D11Device* device, ID3D11DeviceContext* context);

	Handle Get(std::string shaderPath);
	void Revoke(Handle handle);

	void Export(std::string path, Handle handle);
	Handle Load(std::string path);
	const std::string root = "assets/shaders/";

private:
	ID3D11Device* device;
	ID3D11DeviceContext* context;
};

template <class T>
struct ShaderTypeString
{
	static const char* value;
};

typedef ShaderData<SimpleVertexShader> VertexShaderData;
typedef ShaderData<SimplePixelShader> PixelShaderData;