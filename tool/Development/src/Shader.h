#pragma once

#include "Patterns.h"
#include "SimpleShader.h"

template <class T>
class ShaderData : public ProxyHandler<T*, ShaderData<T>>
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