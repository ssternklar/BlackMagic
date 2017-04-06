#pragma once

#include "Patterns.h"
#include "SimpleShader.h"

template<typename T, typename = std::is_base_of<ISimpleShader, T>::type>
struct ShaderTypeString
{
	static const char* value;
};

class ShaderData : public Singleton<ShaderData>
{
public:
	void Init(ID3D11Device* device, ID3D11DeviceContext* context);

	template<typename T>
	T* LoadShader(LPCWSTR path);

private:
	ID3D11Device* device;
	ID3D11DeviceContext* context;
};

template<typename T>
T* ShaderData::LoadShader(LPCWSTR path)
{
	ID3DBlob* blob;
	T* shader = new T(device, context);
	D3DCompileFromFile(path, NULL, NULL, "main", ShaderTypeString<T>::value, 0, 0, &blob, NULL);
	shader->LoadShaderBlob(blob);

	return shader;
}
