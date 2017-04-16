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
	: directory(assetDirectory), _allocator(allocator), renderer(device)
{
}

ContentManager::~ContentManager()
{
}

void ContentManager::AssetGC()
{
	for (int i = 0; i < entryCount; i++)
	{
		if (entries[i].refcount == 0 && entries[i].resource)
		{
			switch (entries[i].type)
			{
			case ManifestEntry::MESH:
				DestructAndDeallocate(_allocator, (Mesh*)entries[i].resource, 1);
				break;
			case ManifestEntry::TEXTURE:
				DestructAndDeallocate(_allocator, (Texture*)entries[i].resource, 1);
				break;
			case ManifestEntry::CUBEMAP:
				DestructAndDeallocate(_allocator, (Cubemap*)entries[i].resource, 1);
				break;
			case ManifestEntry::VERTEX_SHADER:
				DestructAndDeallocate(_allocator, (VertexShader*)entries[i].resource, 1);
				break;
			case ManifestEntry::PIXEL_SHADER:
				DestructAndDeallocate(_allocator, (PixelShader*)entries[i].resource, 1);
				break;
			}
			entries[i].resource = nullptr;
		}
	}
}

struct Asset
{
	uint16_t uid;
	uint16_t filePathIndex;
	uint32_t fileSize;
};

struct ManifestFileHeader
{
	uint16_t pathsSize;
	uint16_t numAssets;
	uint16_t numScenes;
};

void BlackMagic::ContentManager::ProcessManifestFile(void* manifestFileLocation)
{
	ManifestFileHeader* header = (ManifestFileHeader*)manifestFileLocation;
	uint16_t* scenes = (uint16_t*)&(header[1]);
	Asset* assets = (Asset*)&scenes[header->numScenes];
	char* firstString = (char*)&assets[header->numAssets];

	//manipulate scene IDs here

	manifestStrings = (char*)_allocator->allocate(header->pathsSize, 1);
	memcpy_s(manifestStrings, header->pathsSize, firstString, header->pathsSize);
	entries = _allocator->allocate<ManifestEntry>(header->numAssets);
	for (int i = 0; i < header->numAssets; i++)
	{
		entries[i].resourceName = &manifestStrings[assets[i].filePathIndex];
		entries[i].uid = assets[i].uid;
		entries[i].size = assets[i].fileSize;
	}
	entryCount = header->numAssets;
}


template<typename T>
T* ContentManager::load_Internal(const char* fileName, int fileSize)
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
void ContentManager::SetupManifest(ManifestEntry* entry, T* resource)
{
	static_assert(false, "Manifest loading not supported for this type. Supported types are:\n"
		"Mesh\n"
		"Texture\n"
		"VertexShader\n"
		"PixelShader\n"
		"Spline\n"
		);
}

#define LOAD_FILE(X) \
char path[256]; \
memset(path, 0, 256); \
strcpy_s(path, directory); \
strcat_s(path, fileName); \
byte* ##X = (byte*)_allocator->allocate(fileSize); \
if (!PlatformBase::GetSingleton()->ReadFileIntoMemory(path, ##X, fileSize)) \
{ \
	throw "Failed to load file into memory"; \
} \
(void)(sizeof(0))

#define UNLOAD_FILE(X) _allocator->deallocate((void*)##X, fileSize); \
(void)(sizeof(0))

struct MeshHeader
{
	struct Bounds
	{
		float obbCenter[3];
		float emptyCenter;
		float halfSize[3];
		float emptySize;
		float sphere[4];
	};

	struct Block
	{
		uint16_t offsetInBytes;
		uint16_t sizeInBytes;
		uint16_t elementCount;
		uint16_t elementSize;
	};

	uint8_t blockCount;
	Block boundsMeta;
	Block vertexMeta;
	Block indexMeta;
	Bounds bounds;
};

template<>
Mesh* ContentManager::load_Internal(const char* fileName, int fileSize)
{
	LOAD_FILE(meshSpace);

	//block count
	uint8_t blockCount = *meshSpace;
	//Mesh Header Blocks
	MeshHeader::Block* boundsMeta = (MeshHeader::Block*)(meshSpace + 1);
	MeshHeader::Block* vertexMeta = (MeshHeader::Block*)(meshSpace + 1 + (sizeof(MeshHeader::Block)));
	MeshHeader::Block* indexMeta = (MeshHeader::Block*)(meshSpace + 1 + (sizeof(MeshHeader::Block) * 2));

	Mesh* ret = AllocateAndConstruct<BestFitAllocator, Mesh>(_allocator, 1, &meshSpace[vertexMeta->offsetInBytes], vertexMeta->elementCount, &meshSpace[indexMeta->offsetInBytes], indexMeta->elementCount, renderer);
	UNLOAD_FILE(meshSpace);
	return ret;
	
}

template<>
void ContentManager::SetupManifest(ManifestEntry* entry, Mesh* resource)
{
	entry->resource = resource;
	entry->type = ManifestEntry::MESH;
}

template<>
Texture* ContentManager::load_Internal(const char* fileName, int fileSize)
{
	LOAD_FILE(textureSpace);
	Texture* tex = AllocateAndConstruct<BestFitAllocator, Texture>(_allocator, 1, nullptr, nullptr, nullptr, nullptr);
	*tex = renderer->CreateTexture(textureSpace, fileSize, Texture::Type::FLAT_2D, Texture::Usage::READ);
	UNLOAD_FILE(textureSpace);
	return tex;
}

template<>
void ContentManager::SetupManifest(ManifestEntry* entry, Texture* resource)
{
	entry->resource = resource;
	entry->type = ManifestEntry::TEXTURE;
}

template<>
Cubemap* ContentManager::load_Internal(const char* fileName, int fileSize)
{
	LOAD_FILE(textureSpace);
	Texture* tex = AllocateAndConstruct<BestFitAllocator, Texture>(_allocator, 1, nullptr, nullptr, nullptr, nullptr);
	*tex = renderer->CreateTexture(textureSpace, fileSize, Texture::Type::CUBEMAP, Texture::Usage::READ);
	UNLOAD_FILE(textureSpace);
	return (Cubemap*)tex;
}

template<>
void ContentManager::SetupManifest(ManifestEntry* entry, Cubemap* resource)
{
	entry->resource = resource;
	entry->type = ManifestEntry::CUBEMAP;
}

#if defined(BM_PLATFORM_WINDOWS)
using namespace DirectX;

template<>
VertexShader* ContentManager::load_Internal(const char* fileName, int fileSize)
{
	char path[256];
	memset(path, 0, 256);
	strcpy_s(path, directory);
	strcat_s(path, fileName);
	wchar_t widePath[256];
	size_t size = 0;
	mbstowcs_s(&size, widePath, path, 256);
	auto device = reinterpret_cast<DX11Renderer*>(renderer)->Device();
	auto context = reinterpret_cast<DX11Renderer*>(renderer)->Context();
	auto ptr = AllocateAndConstruct<BestFitAllocator, VertexShader>(_allocator, 1, device.Get(), context.Get());
	ptr->LoadShaderFile(widePath);
	return ptr;
}

template<>
void ContentManager::SetupManifest(ManifestEntry* entry, VertexShader* resource)
{
	entry->resource = resource;
	entry->type = ManifestEntry::VERTEX_SHADER;
}

template<>
PixelShader* ContentManager::load_Internal(const char* fileName, int fileSize)
{
	char path[256];
	memset(path, 0, 256);
	strcpy_s(path, directory);
	strcat_s(path, fileName);
	wchar_t widePath[256];
	size_t size = 0;
	mbstowcs_s(&size, widePath, path, 256);
	auto device = reinterpret_cast<DX11Renderer*>(renderer)->Device();
	auto context = reinterpret_cast<DX11Renderer*>(renderer)->Context();
	auto ptr = AllocateAndConstruct<BestFitAllocator, PixelShader>(_allocator, 1, device.Get(), context.Get());
	ptr->LoadShaderFile(widePath);
	return ptr;
}

template<>
void ContentManager::SetupManifest(ManifestEntry* entry, PixelShader* resource)
{
	entry->resource = resource;
	entry->type = ManifestEntry::PIXEL_SHADER;
}

/*
template<>
std::shared_ptr<VertexShader> ContentManager::load_Internal(ManifestEntry* manifest)
{
	auto fullPath = directory + L"/" + name;
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
	auto fullPath = directory + L"/" + name;
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
	auto fullPath = directory + L"/" + name;
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
