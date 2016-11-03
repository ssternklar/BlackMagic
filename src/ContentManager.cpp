#include "ContentManager.h"

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <functional>

#include "WICTextureLoader.h"
#include "Texture.h"
#include "Mesh.h"
#include "Spline.h"

using namespace DirectX;

ContentManager::ContentManager(ID3D11Device* device, ID3D11DeviceContext* ctx, const std::wstring& assetDirectory, BlackMagic::BestFitAllocator* allocator)
	: _device(device), _context(ctx), _assetDirectory(assetDirectory), _allocator(allocator), _resources(ContentMap(ContentAllocatorAdapter(allocator)))
{
}

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

	auto ptr = std::make_shared<Texture>(nullptr, srv, nullptr);
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
		memorySize = sizeof(unsigned int) * 3 + sizeof(SplineControlPoint) * pieces;
		memory = (byte*)_allocator->allocate(memorySize, 1);
		in.seekg(std::ios::beg);
		in.read((char*)memory, memorySize);
		in.close();
	}

	//Fix spline pointers
	Spline* sp = reinterpret_cast<Spline*>(memory);
	sp->segments = reinterpret_cast<SplinePiece*>(memory + 12);

	std::shared_ptr<Spline> ret = //std::allocate_shared<Spline, BlackMagic::AllocatorSTLAdapter<Spline, BlackMagic::BestFitAllocator>>(BlackMagic::AllocatorSTLAdapter<Spline, BlackMagic::BestFitAllocator>(_allocator),
		std::shared_ptr<Spline>((Spline*)memory,
		[&](Spline* splineToDelete) {
		if(splineToDelete)
		{
			_allocator->deallocate((void*)splineToDelete, sizeof(unsigned int) * 3 + sizeof(SplineControlPoint) * splineToDelete->segmentCount, 1);
		}
	}, ContentAllocatorAdapter(_allocator));
		
		//std::bind(&(BlackMagic::BestFitAllocator::deallocate), _allocator, sizeof(unsigned int) + sizeof(SplineControlPoint) * pieces, 1));
	ret.reset((Spline*)memory);
	_resources[name] = ret;
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