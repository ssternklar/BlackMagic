#pragma once

#include <memory>
#include <unordered_map>

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
		T* UntrackedLoad(char* fileName)
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
			for (int i = 0; i < entryCount; i++)
			{
				if (uid == entries[i].uid)
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

		//for compatibility, will be removed later

		template<typename T>
		std::shared_ptr<T> Load(std::string str)
		{
			BestFitAllocator* allocLocal = _allocator;
			T* retLocal = (Load<T>(str.c_str()).get());
			std::shared_ptr<T> ret(retLocal,
				[=](T* foo) {
				//Do nothing, AssetGC takes care of cleanup
			});
			return ret;
		}

		BestFitAllocator* GetAllocator()
		{
			return _allocator;
		}
	};
}