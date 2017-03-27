#pragma once

#include <d3d11.h>
#include <memory>
#include <unordered_map>

#include "SimpleShader.h"

#include <allocators/AllocatorSTLAdapter.h>
#include <allocators/BadBestFitAllocator.h>

#if defined(_WIN32) || defined(_WIN64)
using VertexShader = SimpleVertexShader;
using PixelShader = SimplePixelShader;
#endif

using BlackMagic::IResource;
using ContentAllocatorAdapter = BlackMagic::AllocatorSTLAdapter<std::pair<std::wstring, std::shared_ptr<IResource>>, BlackMagic::BestFitAllocator>;
using ContentMap = std::unordered_map<std::wstring, std::shared_ptr<IResource>, std::hash<std::wstring>, std::equal_to<std::wstring>, ContentAllocatorAdapter>;
namespace BlackMagic
{
	class Renderer;

	class ContentManager
	{
	private:
		struct ManifestEntry
		{
			char* resourceName;
			int uid;
			int size;
			void* resource;
		};
		BlackMagic::BestFitAllocator* _allocator;
		Renderer* renderer;
		ManifestEntry* entries;
		int entryCount;
		const char* directory;

		template<typename T>
		T* load_Internal(ManifestEntry* name);

	public:
		ContentManager(Renderer* device, const char* assetDirectory, BlackMagic::BestFitAllocator* allocator);
		~ContentManager();

		template<typename T>
		T* Load(const char* resourceName)
		{
			for (int i = 0; i < entryCount; i++)
			{
				if (strcmp(resourceName, entries[i].resourceName) == 0)
				{
					if (entries[i].resource)
					{
						return (T*)entries[i].resource;
					}
					else
					{
						entries[i].resource = load_Internal<T>(&entries[i]);
						return (T*)entries[i].resource;
					}
				}
			}
			throw "No file found";
		}

		template<typename T>
		T* Load(int uid)
		{
			for (int i = 0; i < entryCount; i++)
			{
				if (uid == entries[i].uid)
				{
					if (entries[i].resource)
					{
						return entries[i].resource;
					}
					else
					{
						return load_Internal(&entries[i]);
					}
				}
			}
			throw "No file found";
		}

		//for compatibility, will be removed later

		template<typename T>
		std::shared_ptr<T> Load(std::wstring str)
		{
			BestFitAllocator* allocLocal = _allocator;
			T* retLocal = Load<T>((const char*)str.c_str());
			std::shared_ptr<T> ret(retLocal, 
				[allocLocal, retLocal](){
				DestructAndDeallocate<BestFitAllocator, T>(allocLocal, retLocal, 1);
			});
			return ret;
		}

		BestFitAllocator* GetAllocator()
		{
			return _allocator;
		}
	};
}