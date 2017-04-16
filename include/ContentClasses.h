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

	class AssetPointer_Base
	{
	protected:
		ManifestEntry* entry;
	public:
		AssetPointer_Base(ManifestEntry* entry)
		{
			if (entry)
			{
				this->entry = entry;
				entry->refcount++;
			}
		}

		~AssetPointer_Base()
		{
			if (entry)
			{
				entry->refcount--;
			}
		}

		AssetPointer_Base(const AssetPointer_Base& other)
		{
			if (other.entry)
			{
				entry = other.entry;
				entry->refcount++;
			}
		}
	};

	template<typename T>
	class AssetPointer : public AssetPointer_Base
	{
		AssetPointer(ManifestEntry* entry) : AssetPointer_Base(entry) {};
		AssetPointer(const AssetPointer& other) : AssetPointer_Base(other) {};

		T& operator*()
		{
			return *(T*)(entry->resource);
		}

		T* get() const
		{
			return (T*)(entry->resource);
		}

		T* operator->() const
		{
			return (T*)(entry->resource);
		}

		operator bool() const
		{
			return entry != nullptr;
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