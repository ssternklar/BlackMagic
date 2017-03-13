#include "ContentManager.h"

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <functional>

#include "Texture.h"
#include "Mesh.h"
#include "DX11Renderer.h"
#include "DDSTextureLoader.h"

using namespace BlackMagic;

#if defined(_WIN32) || defined(_WIN64)
using namespace DirectX;

ContentManager::ContentManager(Renderer* device, const std::wstring& assetDirectory, BlackMagic::BestFitAllocator* allocator)
	: _assetDirectory(assetDirectory), _allocator(allocator), _resources(ContentMap(ContentAllocatorAdapter(allocator))), renderer(device),
	_adapter(_allocator)
{
}

ContentManager::~ContentManager()
{
	_resources.clear();
}

template<>
std::shared_ptr<Mesh> ContentManager::load_Internal(const std::wstring& name)
{
	auto fullPath = _assetDirectory + L"/" + name;
	auto ptr = std::allocate_shared<Mesh>(AllocatorSTLAdapter<Mesh, BestFitAllocator>(_allocator), fullPath, renderer);
	_resources[name] = ptr;
	return ptr;
}

template<>
std::shared_ptr<Texture> ContentManager::load_Internal(const std::wstring& name)
{
	auto fullPath = _assetDirectory + L"/" + name;
	auto tex = renderer->CreateTexture(fullPath.c_str(), Texture::Type::FLAT, Texture::Usage::READ);
	auto ptr = std::allocate_shared<Texture>(AllocatorSTLAdapter<Texture, BestFitAllocator>(_allocator),  tex);
	_resources[name] = ptr;
	return ptr;
}

template<>
std::shared_ptr<Cubemap> ContentManager::load_Internal(const std::wstring& name)
{
	auto fullPath = _assetDirectory + L"/" + name;
	auto tex = renderer->CreateTexture(fullPath.c_str(), Texture::Type::CUBEMAP, Texture::Usage::READ);
	auto ptr = std::allocate_shared<Cubemap>(AllocatorSTLAdapter<Cubemap, BestFitAllocator>(_allocator), tex);
	_resources[name] = ptr;
	return ptr;
}

template<>
std::shared_ptr<VertexShader> ContentManager::load_Internal(const std::wstring& name)
{
	auto fullPath = _assetDirectory + L"/" + name;
	auto device = reinterpret_cast<DX11Renderer*>(renderer)->Device().Get();
	auto context = reinterpret_cast<DX11Renderer*>(renderer)->Context().Get();
	auto ptr = std::allocate_shared<VertexShader>(ContentAllocatorAdapter(_allocator), 
		device,
		context);
	ptr->LoadShaderFile(fullPath.c_str());
	_resources[name] = ptr;
	return ptr;
}

template<>
std::shared_ptr<PixelShader> ContentManager::load_Internal(const std::wstring& name)
{
	auto fullPath = _assetDirectory + L"/" + name;
	auto device = reinterpret_cast<DX11Renderer*>(renderer)->Device().Get();
	auto context = reinterpret_cast<DX11Renderer*>(renderer)->Context().Get();
	auto ptr = std::allocate_shared<PixelShader>(ContentAllocatorAdapter(_allocator), 
		device,
		context);
	ptr->LoadShaderFile(fullPath.c_str());
	_resources[name] = ptr;
	return ptr;
}

/*
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
*/

#endif

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
