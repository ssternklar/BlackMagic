#pragma once

#include <d3d11.h>
#include <memory>
#include <unordered_map>

#include "SimpleShader.h"

using VertexShader = SimpleVertexShader;
using PixelShader = SimplePixelShader;

class ContentManager
{
public:
	ContentManager(ID3D11Device* device, ID3D11DeviceContext* ctx, const std::wstring& assetDirectory);

	template<typename T>
	std::shared_ptr<T> Load(const std::wstring& name)
	{
		if (_resources.find(name) != _resources.end() && !_resources[name].expired())
			return std::static_pointer_cast<T>(_resources[name].lock());

		return load_Internal<T>(name);
	}

private:
	ID3D11Device* _device;
	ID3D11DeviceContext* _context;
	std::wstring _assetDirectory;
	std::unordered_map<std::wstring, std::weak_ptr<IResource>> _resources;

	template<typename T>
	std::shared_ptr<T> load_Internal(const std::wstring& name);
};