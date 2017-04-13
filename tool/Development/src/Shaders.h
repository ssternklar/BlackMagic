#pragma once

#include "Patterns.h"
#include "SimpleShader.h"
#include "StringManip.h"

struct Shader
{
	friend class ShaderData;
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

	template<Shader::Type> struct TypeMap;
	template<> struct TypeMap<Shader::Compute> { using type = SimpleComputeShader; };
	template<> struct TypeMap<Shader::Domain> { using type = SimpleDomainShader; };
	template<> struct TypeMap<Shader::Geometry> { using type = SimpleComputeShader; };
	template<> struct TypeMap<Shader::Hull> { using type = SimpleHullShader; };
	template<> struct TypeMap<Shader::Pixel> { using type = SimplePixelShader; };
	template<> struct TypeMap<Shader::Vertex> { using type = SimpleVertexShader; };

	template <Type T>
	typename TypeMap<T>::type* GetAs()
	{
		assert(T == type);
		return dynamic_cast<TypeMap<T>::type*>(shader);
	}

private:
	ISimpleShader* shader;
	Type type;
};

class ShaderData : public ProxyHandler<Shader, ShaderData>
{
public:
	~ShaderData();

	void Init(ID3D11Device* device, ID3D11DeviceContext* context);

	Handle Get(std::string shaderPath, Shader::Type type);
	void Revoke(Handle handle);

	void Export(std::string path, Handle handle);
	template <Shader::Type T>
	Handle Load(std::string path);
	const std::string root = "assets/shaders/";

private:
	ID3D11Device* device;
	ID3D11DeviceContext* context;
};

template <Shader::Type T>
struct ShaderTypeString
{
	static const char* value;
};

template <Shader::Type T>
ShaderData::Handle ShaderData::Load(std::string path)
{
	using type = typename Shader::TypeMap<T>::type;
	wstring wpath = StringManip::utf8_decode(path);

	ID3DBlob* blob;
	type* shader = new type(device, context);
	D3DCompileFromFile(wpath.c_str(), NULL, NULL, "main", ShaderTypeString<T>::value, 0, 0, &blob, NULL);
	shader->LoadShaderBlob(blob);

	if (!shader)
	{
		Handle e;
		return e;
	}

	Handle h = ProxyHandler::Get();

	h->shader = shader;
	h->type = T;

	return h;
}
