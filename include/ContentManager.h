#pragma once

#include <d3d11.h>
#include <memory>
#include <unordered_map>

#include "SimpleShader.h"

#include <allocators/AllocatorSTLAdapter.h>
#include <allocators/BadBestFitAllocator.h>

using VertexShader = SimpleVertexShader;
using PixelShader = SimplePixelShader;
using ContentAllocatorAdapter = BlackMagic::AllocatorSTLAdapter<std::pair<std::wstring, std::weak_ptr<IResource>>, BlackMagic::BestFitAllocator>;
using ContentMap = std::unordered_map<std::wstring, std::weak_ptr<IResource>, std::hash<std::wstring>, std::equal_to<std::wstring>, ContentAllocatorAdapter>;
namespace BlackMagic
{
	class GraphicsDevice;
	class ContentManager
	{
	public:
		ContentManager(GraphicsDevice* device, const std::wstring& assetDirectory, BlackMagic::BestFitAllocator* allocator);

		template<typename T>
		std::shared_ptr<T> Load(const std::wstring& name)
		{
			if (_resources.find(name) != _resources.end() && !_resources[name].expired())
				return std::static_pointer_cast<T>(_resources[name].lock());

			return load_Internal<T>(name);
		}

		BestFitAllocator* GetAllocator()
		{
			return _allocator;
		}

	private:
		ID3D11Device* _device;
		ID3D11DeviceContext* _context;
		std::wstring _assetDirectory;
		ContentMap _resources;
		BlackMagic::BestFitAllocator* _allocator;
	

		template<typename T>
		std::shared_ptr<T> load_Internal(const std::wstring& name);
	};
}