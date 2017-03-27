#include "ContentManager.h"

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <functional>

#include "PlatformBase.h"
#include "GraphicsTypes.h"
#include "Texture.h"
#include "Mesh.h"
#include "DX11Renderer.h"
#include "DDSTextureLoader.h"

using namespace BlackMagic;

ContentManager::ContentManager(Renderer* device, const char* assetDirectory, BlackMagic::BestFitAllocator* allocator)
{
	renderer = device;
	directory = assetDirectory;
	_allocator = allocator;
}

ContentManager::~ContentManager()
{
}


template<typename T>
T* ContentManager::load_Internal(ManifestEntry* manifest)
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

template<typename T>
T ContentManager::loadHandle_Internal(ManifestEntry* manifest)
{
	static_assert(false,
		"Invalid or unsupported content type provided. Supported types are:\n"
		"GraphicsShader\n"
		);
}

#define LOAD_FILE(X) \
char path[256]; \
memset(path, 0, 256); \
strcpy_s(path, directory); \
strcat_s(path, manifest->resourceName); \
byte* ##X = (byte*)_allocator->allocate(manifest->size); \
if (!PlatformBase::GetSingleton()->ReadFileIntoMemory(path, ##X, manifest->size)) \
{ \
	throw "Failed to load file into memory"; \
} \
(void)(sizeof(0))

#define UNLOAD_FILE(X) _allocator->deallocate(##X); \
(void)(sizeof(0))

struct MeshHeader
{
	int offsetToVertexData;
	int vertexDataCount;
	int offsetToIndexData;
	int indexDataCount;
};

template<>
Mesh* ContentManager::load_Internal(ManifestEntry* manifest)
{
	LOAD_FILE(meshSpace);
	MeshHeader* header = (MeshHeader*)meshSpace;
	Mesh* ret = AllocateAndConstruct<BestFitAllocator, Mesh>(_allocator, 1, &meshSpace[header->offsetToVertexData], header->vertexDataCount, &meshSpace[header->offsetToIndexData], header->indexDataCount, renderer);
	UNLOAD_FILE(meshSpace);
	return ret;
	
}

template<>
Texture* ContentManager::load_Internal(ManifestEntry* manifest)
{
	LOAD_FILE(textureSpace);
	GraphicsTexture tex = renderer->CreateTexture(textureSpace, manifest->size, GraphicsTexture::TextureType::FLAT);
	Texture* ret = AllocateAndConstruct<BestFitAllocator, Texture>(_allocator, 1, renderer, tex, nullptr);
	UNLOAD_FILE(textureSpace);
}

template<>
Cubemap* ContentManager::load_Internal(ManifestEntry* manifest)
{
	LOAD_FILE(textureSpace);
	GraphicsTexture tex = renderer->CreateTexture(textureSpace, manifest->size, GraphicsTexture::TextureType::CUBEMAP);
	Texture* ret = AllocateAndConstruct<BestFitAllocator, Texture>(_allocator, 1, renderer, tex, nullptr);
	UNLOAD_FILE(textureSpace);
}

/*template<>
GraphicsShader ContentManager::loadHandle_Internal(ManifestEntry* manifest)
{
	LOAD_FILE(shaderSpace);
	UNLOAD_FILE(shaderSpace);
}*/

#if defined(_WIN32) || defined(_WIN64)
using namespace DirectX;

template<>
VertexShader* ContentManager::load_Internal(ManifestEntry* manifest)
{
	char path[256];
	memset(path, 0, 256);
	strcpy_s(path, directory);
	strcat_s(path, manifest->resourceName);
	auto device = reinterpret_cast<DX11Renderer*>(renderer)->Device();
	auto context = reinterpret_cast<DX11Renderer*>(renderer)->Context();
	auto ptr = AllocateAndConstruct<BestFitAllocator, VertexShader>(_allocator, 1, device.Get(), context.Get());
	ptr->LoadShaderFile((LPCWSTR)path);
	manifest->resource = ptr;
	return ptr;
}

template<>
PixelShader* ContentManager::load_Internal(ManifestEntry* manifest)
{
	char path[256];
	memset(path, 0, 256);
	strcpy_s(path, directory);
	strcat_s(path, manifest->resourceName);
	auto device = reinterpret_cast<DX11Renderer*>(renderer)->Device();
	auto context = reinterpret_cast<DX11Renderer*>(renderer)->Context();
	auto ptr = AllocateAndConstruct<BestFitAllocator, PixelShader>(_allocator, 1, device.Get(), context.Get());
	ptr->LoadShaderFile((LPCWSTR)path);
	manifest->resource = ptr;
	return ptr;
}

/*
template<>
std::shared_ptr<VertexShader> ContentManager::load_Internal(ManifestEntry* manifest)
{
	auto fullPath = _assetDirectory + L"/" + name;
	auto device = reinterpret_cast<DX11Renderer*>(renderer)->Device();
	auto context = reinterpret_cast<DX11Renderer*>(renderer)->Context();
	auto ptr = std::allocate_shared<VertexShader>(ContentAllocatorAdapter(_allocator), 
		device.Get(),
		context.Get());
	ptr->LoadShaderFile(fullPath.c_str());
	_resources[name] = ptr;
	return ptr;
}

template<>
std::shared_ptr<PixelShader> ContentManager::load_Internal(const std::wstring& name)
{
	auto fullPath = _assetDirectory + L"/" + name;
	auto device = reinterpret_cast<DX11Renderer*>(renderer)->Device();
	auto context = reinterpret_cast<DX11Renderer*>(renderer)->Context();
	auto ptr = std::allocate_shared<PixelShader>(ContentAllocatorAdapter(_allocator), 
		device.Get(),
		context.Get());
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
}*/


#endif
/*
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
}*/
