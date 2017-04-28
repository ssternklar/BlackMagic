#include "ContentManager.h"

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <functional>

#include "PlatformBase.h"
#include "GraphicsTypes.h"
#include "Texture.h"
#include "Mesh.h"
#include "WAVFile.h"

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
		if (BM_PLATFORM_ATOMIC_FETCH((&(entries[i].refcount))) == 0 && entries[i].resource)
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
			case ManifestEntry::WAVFILE:
				((WAVFile*)(entries[i].resource))->~WAVFile();
				_allocator->deallocate(entries[i].resource, sizeof(WAVFile) + entries[i].size);
				break;
			default:
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
	assert(false);
	/*
		"Invalid or unsupported content type provided. Supported types are:\n"
		"Mesh\n"
		"Texture\n"
		"VertexShader\n"
		"PixelShader\n"
		"Spline\n"
	*/
}

template<typename T>
void ContentManager::SetupManifest(ManifestEntry* entry, T* resource)
{
	assert(false);
	/*
		"Manifest loading not supported for this type. Supported types are:\n"
		"Mesh\n"
		"Texture\n"
		"VertexShader\n"
		"PixelShader\n"
		"Spline\n"
	*/
}

struct AssetFile
{
	BlackMagic::byte* memory;
	BlackMagic::BestFitAllocator* allocator;
	AssetFile(const char* dir, const char* name, BlackMagic::BestFitAllocator* allocator)
		: allocator(allocator)
	{
		auto platform = PlatformBase::GetSingleton();
		char path[256] = { 0 };
		strcpy_s(path, dir);
		strcat_s(path, name);
		auto size = platform->GetFileSize(path);

		memory = static_cast<BlackMagic::byte*>(allocator->allocate(size));
		if (!platform->ReadFileIntoMemory(path, memory, size))
		{
			assert(false);// "Failed to load file into memory";
		}
	}

	~AssetFile() { allocator->deallocate(memory); }
};

#define LOAD_FILE(X) \
char path[256]; \
memset(path, 0, 256); \
strcpy_s(path, directory); \
strcat_s(path, fileName); \
byte* ##X = (byte*)_allocator->allocate(fileSize); \
if (!PlatformBase::GetSingleton()->ReadFileIntoMemory(path, ##X, fileSize)) \
{ \
	assert(false);// "Failed to load file into memory"; \
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
	auto file = AssetFile{ directory, fileName, _allocator };
	auto meshSpace = file.memory;

	//block count
	uint8_t blockCount = *meshSpace;
	//Mesh Header Blocks
	MeshHeader::Block* boundsMeta = (MeshHeader::Block*)(meshSpace + 1);
	MeshHeader::Block* vertexMeta = (MeshHeader::Block*)(meshSpace + 1 + (sizeof(MeshHeader::Block)));
	MeshHeader::Block* indexMeta = (MeshHeader::Block*)(meshSpace + 1 + (sizeof(MeshHeader::Block) * 2));

	Mesh* ret = AllocateAndConstruct<Mesh>(_allocator, 1, &meshSpace[vertexMeta->offsetInBytes], vertexMeta->elementCount, &meshSpace[indexMeta->offsetInBytes], indexMeta->elementCount, renderer);
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
	auto file = AssetFile{ directory, fileName, _allocator };
	auto textureSpace = file.memory;
	Texture* tex = AllocateAndConstruct<Texture>(_allocator, 1, nullptr, nullptr, nullptr, nullptr);
	*tex = renderer->CreateTexture(textureSpace, fileSize, Texture::Type::FLAT_2D, Texture::Usage::READ);
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
	auto file = AssetFile{ directory, fileName, _allocator };
	auto textureSpace = file.memory;
	Texture* tex = AllocateAndConstruct<Texture>(_allocator, 1, nullptr, nullptr, nullptr, nullptr);
	*tex = renderer->CreateTexture(textureSpace, fileSize, Texture::Type::CUBEMAP, Texture::Usage::READ);
	return (Cubemap*)tex;
}

template<>
void ContentManager::SetupManifest(ManifestEntry* entry, Cubemap* resource)
{
	entry->resource = resource;
	entry->type = ManifestEntry::CUBEMAP;
}

template<>
WAVFile* ContentManager::load_Internal(const char* fileName, int fileSize)
{
	char path[256];
	memset(path, 0, 256);
	strcpy_s(path, directory);
	strcat_s(path, fileName);
	byte* audioSpace = (byte*)_allocator->allocate(sizeof(WAVFile) + fileSize);
	byte* audioFile = audioSpace + sizeof(WAVFile);
	PlatformBase::GetSingleton()->ReadFileIntoMemory(path, audioFile, fileSize);
	WAVFile* wav = new (audioSpace) WAVFile(audioFile);
	return wav;
}

template<>
void ContentManager::SetupManifest(ManifestEntry* entry, WAVFile* resource)
{
	entry->resource = resource;
	entry->type = ManifestEntry::WAVFILE;
}

#if defined(BM_PLATFORM_WINDOWS)
#include "DX11Renderer.h"
#endif

template<>
VertexShader* ContentManager::load_Internal(const char* fileName, int fileSize)
{
	char path[256];
	memset(path, 0, 256);
	strcpy_s(path, directory);
	strcat_s(path, fileName);
#if defined(BM_PLATFORM_WINDOWS)
	wchar_t widePath[256];
	size_t size = 0;
	mbstowcs_s(&size, widePath, path, 256);
	auto device = reinterpret_cast<DX11Renderer*>(renderer)->Device();
	auto context = reinterpret_cast<DX11Renderer*>(renderer)->Context();
	auto ptr = AllocateAndConstruct<VertexShader>(_allocator, 1, device.Get(), context.Get());
	ptr->LoadShaderFile(widePath);
#else
	auto ptr = AllocateAndConstruct<VertexShader>(_allocator, 1, renderer);
	ptr->LoadShaderFile(path);
#endif
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
#if defined(BM_PLATFORM_WINDOWS)
	wchar_t widePath[256];
	size_t size = 0;
	mbstowcs_s(&size, widePath, path, 256);
	auto device = reinterpret_cast<DX11Renderer*>(renderer)->Device();
	auto context = reinterpret_cast<DX11Renderer*>(renderer)->Context();
	auto ptr = AllocateAndConstruct<PixelShader>(_allocator, 1, device.Get(), context.Get());
	ptr->LoadShaderFile(widePath);
#else
	auto ptr = AllocateAndConstruct<PixelShader>(_allocator, 1, renderer);
	ptr->LoadShaderFile(path);
#endif
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

