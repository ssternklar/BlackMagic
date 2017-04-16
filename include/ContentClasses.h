#pragma once

#include <atomic>

namespace BlackMagic
{
	struct ManifestEntry
	{
		enum ResourceType
		{
			MESH,
			TEXTURE,
			CUBEMAP,
			VERTEX_SHADER,
			PIXEL_SHADER,
		};
		char* resourceName;
		int uid;
		int size;
		void* resource;
		std::atomic_int refcount = 0;
		ResourceType type;
	};

	template<typename T>
	class AssetPointer
	{
	private:
		ManifestEntry* entry;
	public:
		AssetPointer(ManifestEntry* entry)
		{
			if (entry)
			{
				this->entry = entry;
				entry->refcount++;
			}
		}

		~AssetPointer()
		{
			if (entry)
			{
				entry->refcount--;
			}
		}

		AssetPointer(const AssetPointer& other)
		{
			if (other.entry)
			{
				entry = other->entry;
				entry->refcount++;
			}
		}

		T& operator*()
		{
			return *(T*)(entry->resource);
		}

		T* get()
		{
			return (T*)(entry->resource);
		}

		T* operator->()
		{
			return (T*)(entry->resource);
		}

		void reset()
		{
			if (entry)
			{
				entry->refcount--;
			}
		}

	};
}