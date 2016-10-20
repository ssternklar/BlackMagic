#include "ContentManager.h"

#include <stdexcept>

#include "WICTextureLoader.h"
#include "Texture.h"
#include "Mesh.h"

using namespace DirectX;

ContentManager::ContentManager(ID3D11Device* device, ID3D11DeviceContext* ctx, const std::wstring& assetDirectory)
	: _device(device), _context(ctx), _assetDirectory(assetDirectory)
{}

template<>
std::shared_ptr<Mesh> ContentManager::load_Internal(const std::wstring& name)
{
	auto fullPath = _assetDirectory + L"/" + name;
	auto ptr = std::make_shared<Mesh>(fullPath, _device);
	_resources[name] = ptr;
	return ptr;
}

template<>
std::shared_ptr<Texture> ContentManager::load_Internal(const std::wstring& name)
{
	ID3D11ShaderResourceView* srv;
	auto fullPath = _assetDirectory + L"/" + name;
	auto result = CreateWICTextureFromFile(_device, _context, fullPath.c_str(), nullptr, &srv);

	auto ptr = std::make_shared<Texture>(srv);
	_resources[name] = ptr;
	return ptr;
}

template<>
std::shared_ptr<VertexShader> ContentManager::load_Internal(const std::wstring& name)
{
	auto fullPath = _assetDirectory + L"/" + name;
	auto ptr = std::make_shared<VertexShader>(_device, _context);
	ptr->LoadShaderFile(fullPath.c_str());
	_resources[name] = ptr;
	return ptr;
}

template<>
std::shared_ptr<PixelShader> ContentManager::load_Internal(const std::wstring& name)
{
	auto fullPath = _assetDirectory + L"/" + name;
	auto ptr = std::make_shared<PixelShader>(_device, _context);
	ptr->LoadShaderFile(fullPath.c_str());
	_resources[name] = ptr;
	return ptr;
}

template<typename T>
std::shared_ptr<T> ContentManager::load_Internal(const std::wstring& name)
{
	static_assert(false,
		"Invalid or unsupported content type provided. Supported types are:\n"
		"Mesh\n"
		"Texture\n"
		"VertexShader\n"
		"PixelShader\n"
	);
}