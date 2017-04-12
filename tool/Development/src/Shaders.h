#pragma once

#include "Patterns.h"
#include "SimpleShader.h"

struct Shader
{
public:
	enum Type
	{
		Compute,
		Domain,
		Geometry,
		Hull,
		Pixel,
		Vertex
	};

	// how to return type based on enum?
	ISimpleShader* operator->() { return shader; }
	ISimpleShader& operator *() { return *shader; }

private:
	ISimpleShader* shader;
	Type type;
};

class ShaderData : public ProxyHandler<Shader, ShaderData>
{
public:
	void Init(ID3D11Device* device, ID3D11DeviceContext* context);

	template <typename T>
	T* Load(LPCWSTR path);

private:
	ID3D11Device* device;
	ID3D11DeviceContext* context;
};

template <typename T, typename = std::is_base_of<ISimpleShader, T>::type>
struct ShaderTypeString
{
	static const char* value;
};

template <typename T>
T* ShaderData::Load(LPCWSTR path)
{
	ID3DBlob* blob;
	T* shader = new T(device, context);
	D3DCompileFromFile(path, NULL, NULL, "main", ShaderTypeString<T>::value, 0, 0, &blob, NULL);
	shader->LoadShaderBlob(blob);

	return shader;
}
