#include "Shader.h"
#include "StringManip.h"
#include "Assets.h"
#include "FileUtil.h"

const char* ShaderTypeString<SimpleComputeShader>::value = "cs_5_0";
const char* ShaderTypeString<SimpleDomainShader>::value = "ds_5_0";
const char* ShaderTypeString<SimpleGeometryShader>::value = "gs_5_0";
const char* ShaderTypeString<SimpleHullShader>::value = "hs_5_0";
const char* ShaderTypeString<SimplePixelShader>::value = "ps_5_0";
const char* ShaderTypeString<SimpleVertexShader>::value = "vs_5_0";

template <class T>
ShaderData<T>::~ShaderData()
{
	for (size_t i = 0; i < size; ++i)
		delete data[i];
}

template <class T>
void ShaderData<T>::Init(ID3D11Device* device, ID3D11DeviceContext* context)
{
	this->device = device;
	this->context = context;
}

template <class T>
typename ShaderData<T>::Handle ShaderData<T>::Get(std::string shaderPath)
{
	std::string fullPath = root + shaderPath;

	Handle h = AssetManager::Instance().GetHandle<ShaderData<T>>(fullPath);
	if (h.ptr())
		return h;

	h = Load(fullPath);

	if (!h.ptr())
		return h;

	AssetManager::Instance().TrackAsset<ShaderData<T>>(h, fullPath);

	return h;
}

template <class T>
void ShaderData<T>::Revoke(typename ShaderData<T>::Handle handle)
{
	delete handle.ptr();

	AssetManager::Instance().StopTrackingAsset<ShaderData<T>>(handle);
	ProxyHandler::Revoke(handle);
}

template <class T>
void ShaderData<T>::Export(std::string path, typename ShaderData<T>::Handle handle)
{
	FileUtil::CopyFileViaPaths(AssetManager::Instance().GetAsset<ShaderData<T>>(handle).path, path);
}

template <class T>
typename ShaderData<T>::Handle ShaderData<T>::Load(std::string path)
{
	Handle e;

	if (!FileUtil::DoesfileExist(path))
		return e;

	wstring wpath = StringManip::utf8_decode(path);

	ID3DBlob* blob;
	T* shader = new T(device, context);
	D3DCompileFromFile(wpath.c_str(), NULL, NULL, "main", ShaderTypeString<T>::value, 0, 0, &blob, NULL);

	if (!blob)
		return e;

	shader->LoadShaderBlob(blob);

	if (!shader)
		return e;

	Handle h = ProxyHandler::Get();

	data[size-1] = shader;

	return h;
}

template class ShaderData<SimpleVertexShader>;
template class ShaderData<SimplePixelShader>;