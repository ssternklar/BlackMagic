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
#include "Scene.h"

using namespace BlackMagic;

ContentManager::ContentManager(Renderer* device, const char* assetDirectory, BlackMagic::BestFitAllocator* allocator)
	: directory(assetDirectory), _allocator(allocator), renderer(device)
{
}

ContentManager::~ContentManager()
{
}

void ContentManager::ForceAssetCleanup(ManifestEntry* entry)
{
	if (entry->resource)
	{
		switch (entry->type)
		{
		case ManifestEntry::MESH:
			DestructAndDeallocate(_allocator, (Mesh*)entry->resource, 1);
			break;
		case ManifestEntry::TEXTURE:
			DestructAndDeallocate(_allocator, (Texture*)entry->resource, 1);
			break;
		case ManifestEntry::CUBEMAP:
			DestructAndDeallocate(_allocator, (Cubemap*)entry->resource, 1);
			break;
		case ManifestEntry::VERTEX_SHADER:
			DestructAndDeallocate(_allocator, (VertexShader*)entry->resource, 1);
			break;
		case ManifestEntry::PIXEL_SHADER:
			DestructAndDeallocate(_allocator, (PixelShader*)entry->resource, 1);
			break;
		case ManifestEntry::WAVFILE:
			((WAVFile*)(entry->resource))->~WAVFile();
			_allocator->deallocate(entry->resource, sizeof(WAVFile) + entry->size);
			break;
		case ManifestEntry::SCENE:
			_allocator->deallocate(((void*)((SceneDesc*)entry->resource)->fileHandle), entry->size);
			DestructAndDeallocate(_allocator, (SceneDesc*)entry->resource, 1);
		default:
			break;
		}
		entry->resource = nullptr;
	}
}

void ContentManager::AssetGC()
{
	for (int i = 0; i < entryCount; i++)
	{
		if (BM_PLATFORM_ATOMIC_FETCH((&(entries[i].refcount))) == 0)
		{
			ForceAssetCleanup(entries + i);
		}
	}
}

ManifestEntry* BlackMagic::ContentManager::GetManifestByUID(int uid)
{
	for (int i = 0; i < entryCount; i++)
	{
		if (uid == entries[i].uid)
		{
			return &entries[i];
		}
	}
	return nullptr;
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
	assert(false); \
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

template<>
SceneDesc* ContentManager::load_Internal(const char* fileName, int fileSize)
{
	LOAD_FILE(sceneMemory);
	uint16_t* sceneStuff = (uint16_t*)sceneMemory;
	uint16_t uidCount = *sceneStuff;
	uint16_t* uids = ++sceneStuff;
	sceneStuff += uidCount;
	uint16_t entityCount = *sceneStuff;
	byte* entities = (byte*)(++sceneStuff);
	SceneDesc* ret = AllocateAndConstruct<SceneDesc>(_allocator, 1, sceneMemory);
	return ret;
}

template<>
Material* ContentManager::load_Internal(const char* fileName, int fileSize)
{
	//auto file = AssetFile{ directory, fileName, _allocator };
	assert(false);
	return nullptr;
}

template<>
void ContentManager::SetupManifest(ManifestEntry* entry, Material* resource)
{
	assert(false);

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

ManifestEntry::ResourceType GetTypeFromFileName(const char* fileName)
{
	if (strstr(fileName, ".bmmesh"))
	{
		return ManifestEntry::MESH;
	}
	else if (strstr(fileName, ".wav"))
	{
		return ManifestEntry::WAVFILE;
	}
	else if (strstr(fileName, ".png"))
	{
		return ManifestEntry::TEXTURE;
	}
	else if (strstr(fileName, ".bmspline"))
	{
		return ManifestEntry::SPLINE;
	}
	else
	{
		//Can't deduce type from file extension alone
		assert(false);
	}
}

template<>
UnknownContentType* ContentManager::load_Internal(const char* fileName, int fileSize)
{
	auto type = GetTypeFromFileName(fileName);
	switch (type)
	{
	case ManifestEntry::MESH:
		return reinterpret_cast<UnknownContentType*>(load_Internal<Mesh>(fileName, fileSize));
		break;
	case ManifestEntry::WAVFILE:
		return reinterpret_cast<UnknownContentType*>(load_Internal<WAVFile>(fileName, fileSize));
		break;
	case ManifestEntry::TEXTURE:
		return reinterpret_cast<UnknownContentType*>(load_Internal<Texture>(fileName, fileSize));
		break;
	case ManifestEntry::SPLINE:
		//return load_Internal<Spline>(fileName, fileSize);
		//break;
	default:
		//Can't deduce type from file extension alone
		assert(false);
		break;
	}
	return nullptr;
}

template<>
void ContentManager::SetupManifest(ManifestEntry* entry, UnknownContentType* resource)
{
	entry->resource = resource;
	entry->type = GetTypeFromFileName(entry->resourceName);
}
