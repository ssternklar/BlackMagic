#pragma once

#include <memory>

#include "PlatformBase.h"
#include "SimpleShader.h"

#include "ContentClasses.h"
#include <allocators/AllocatorSTLAdapter.h>
#include <allocators/BadBestFitAllocator.h>


namespace BlackMagic
{
	class Renderer;

	class ContentManager
	{
	private:
		BlackMagic::BestFitAllocator* _allocator;
		Renderer* renderer;
		char* manifestStrings;
		ManifestEntry* entries;
		int entryCount;
		const char* directory;

		template<typename T>
		T* load_Internal(const char* fileName, int fileSize);
		template<typename T>
		void SetupManifest(ManifestEntry* entry, T* resource);

	public:
		ContentManager(Renderer* device, const char* assetDirectory, BlackMagic::BestFitAllocator* allocator);
		~ContentManager();

		void ProcessManifestFile(void* manifestFileLocation);

		template<typename T>
		T* UntrackedLoad(const char* fileName)
		{
			char path[256] = { 0 };
			strcpy_s(path, directory);
			strcat_s(path, fileName);
			int fileSize = PlatformBase::GetSingleton()->GetFileSize(path);
			return load_Internal<T>(fileName, fileSize);
		}

		template<typename T>
		void UntrackedAssetCleanup(T* thing)
		{
			DestructAndDeallocate<T>(_allocator, thing, 1);
		}

		void ForceAssetCleanup(ManifestEntry* entry);
		void AssetGC();

		template<typename T>
		AssetPointer<T> Load(const char* resourceName)
		{
			for (int i = 0; i < entryCount; i++)
			{
				if (strcmp(resourceName, entries[i].resourceName) == 0)
				{
					if (entries[i].resource)
					{
						return AssetPointer<T>(&entries[i]);
					}
					else
					{
						SetupManifest(&entries[i], load_Internal<T>(entries[i].resourceName, entries[i].size));
						return AssetPointer<T>(&entries[i]);
					}
				}
			}
			assert(false); // No file found
			return nullptr;
		}

		template<typename T>
		AssetPointer<T> Load(int uid)
		{
			ManifestEntry* entry = GetManifestByUID(uid);
			if (strstr(entry->resourceName, "defaults/"))
			{
				return nullptr;
			}
			if (entry->resource)
			{
				return AssetPointer<T>(entry);
			}
			else
			{
				T* resource = load_Internal<T>(entry->resourceName, entry->size);
				SetupManifest<T>(entry, resource);
				return AssetPointer<T>(entry);
			}
			assert(false); // No file found
			return nullptr;
		}

		ManifestEntry* GetManifestByUID(int uid);

		//for compatibility, will be removed later

		template<typename T>
		std::shared_ptr<T> ConvertToSharedPtr(AssetPointer<T> assetPtr)
		{
			ManifestEntry* entry = assetPtr.entry;
			BM_PLATFORM_ATOMIC_ADD(&entry->refcount, 1);
			std::shared_ptr<T> ret(assetPtr.get(),
				[=](T* foo) {
				BM_PLATFORM_ATOMIC_ADD(&entry->refcount, -1);
			});
			return ret;
		}

		template<typename T>
		std::shared_ptr<T> Load(std::string str)
		{
			BestFitAllocator* allocLocal = _allocator;
			std::shared_ptr<T> ret = ConvertToSharedPtr<T>(Load<T>(str.c_str()));
			return ret;
		}
		
		BestFitAllocator* GetAllocator()
		{
			return _allocator;
		}
	};
}