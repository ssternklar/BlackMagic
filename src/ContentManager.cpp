#include "ContentManager.h"

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <functional>

#include "Texture.h"
#include "Mesh.h"
#include "Spline.h"
#include "DirectXGraphicsDevice.h"
#include "DDSTextureLoader.h"

using namespace BlackMagic;
using namespace DirectX;

ContentManager::ContentManager(GraphicsDevice* device, const std::wstring& assetDirectory, BlackMagic::BestFitAllocator* allocator)
	: _assetDirectory(assetDirectory), _allocator(allocator), _resources(ContentMap(ContentAllocatorAdapter(allocator))), graphicsDevice(device)
{
	_device = ((DirectXGraphicsDevice*)device)->Device();
	_context = ((DirectXGraphicsDevice*)device)->Context();
}

template<>
std::shared_ptr<Mesh> ContentManager::load_Internal(const std::wstring& name)
{
	auto fullPath = _assetDirectory + L"/" + name;
	auto ptr = std::allocate_shared<Mesh>(ContentAllocatorAdapter(_allocator), fullPath, graphicsDevice);
	_resources[name] = ptr;
	return ptr;
}

template<>
std::shared_ptr<Texture> ContentManager::load_Internal(const std::wstring& name)
{
	auto fullPath = _assetDirectory + L"/" + name;

	GraphicsTexture tex = graphicsDevice->CreateTexture((const char*)fullPath.c_str());

	auto ptr = std::allocate_shared<Texture>(AllocatorSTLAdapter<Texture, BestFitAllocator>(_allocator), graphicsDevice, tex, GraphicsRenderTarget(nullptr));
	_resources[name] = ptr;
	return ptr;
}

template<>
std::shared_ptr<Cubemap> ContentManager::load_Internal(const std::wstring& name)
{
	ID3D11ShaderResourceView* srv;
	auto fullPath = _assetDirectory + L"/" + name;
	auto result = CreateDDSTextureFromFile(_device, _context, fullPath.c_str(), nullptr, &srv);
	auto ptr = std::allocate_shared<Cubemap>(AllocatorSTLAdapter<Cubemap, BestFitAllocator>(_allocator), graphicsDevice, GraphicsTexture(srv), GraphicsRenderTarget(nullptr));;
	_resources[name] = ptr;
	return ptr;
}

template<>
std::shared_ptr<VertexShader> ContentManager::load_Internal(const std::wstring& name)
{
	auto fullPath = _assetDirectory + L"/" + name;
	auto ptr = std::allocate_shared<VertexShader>(ContentAllocatorAdapter(_allocator), _device, _context);
	ptr->LoadShaderFile(fullPath.c_str());
	_resources[name] = ptr;
	return ptr;
}

template<>
std::shared_ptr<PixelShader> ContentManager::load_Internal(const std::wstring& name)
{
	auto fullPath = _assetDirectory + L"/" + name;
	auto ptr = std::allocate_shared<PixelShader>(ContentAllocatorAdapter(_allocator), _device, _context);
	ptr->LoadShaderFile(fullPath.c_str());
	_resources[name] = ptr;
	return ptr;
}

template<>
std::shared_ptr<Spline> ContentManager::load_Internal(const std::wstring& name)
{
	auto alloc = _resources.get_allocator();
	auto fullPath = _assetDirectory + L"/" + name;
	std::ifstream in(fullPath, std::ios::binary);
	unsigned int pieces = 0;
	size_t memorySize = 0;
	byte* memory = 0;
	if (in.is_open())
	{
		in.read((char*)&pieces, 4);
		memorySize = sizeof(unsigned int) * 4 + sizeof(SplinePiece) * pieces;
		memory = (byte*)_allocator->allocate(memorySize, 1);
		in.seekg(std::ios::beg);
		in.read((char*)memory, memorySize);
		in.close();
	}

	//Fix spline pointers
	Spline* sp = reinterpret_cast<Spline*>(memory);
	sp->segments = reinterpret_cast<SplinePiece*>(memory + 16);

	std::shared_ptr<Spline> ret =
		std::shared_ptr<Spline>(sp,
			[&](Spline* splineToDelete) {
		if (splineToDelete)
		{
			_allocator->deallocate((void*)splineToDelete, sizeof(unsigned int) * 4 + sizeof(SplineControlPoint) * splineToDelete->segmentCount, 1);
		}
	}, BlackMagic::AllocatorSTLAdapter<Spline, BlackMagic::BestFitAllocator>(_allocator));

	//_resources[name] = ret;
	return ret;
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
		"Spline\n"
		);
}