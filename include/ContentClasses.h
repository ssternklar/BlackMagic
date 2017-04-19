#pragma once

#include BM_PLATFORM_ATOMIC_LIBRARY

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
		volatile unsigned int refcount = 0;
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
				BM_PLATFORM_ATOMIC_INCREMENT(&(entry->refcount));
			}
		}

		~AssetPointer_Base()
		{
			reset();
		}

		AssetPointer_Base(const AssetPointer_Base& other)
		{
			if (other.entry)
			{
				entry = other.entry;
				BM_PLATFORM_ATOMIC_INCREMENT(&(entry->refcount));
			}
		}

		void reset()
		{
			if (entry)
			{
				BM_PLATFORM_ATOMIC_DECREMENT(&(entry->refcount));
			}
		}

		operator bool() const
		{
			return entry != nullptr;
		}
	};

	template<typename T>
	class AssetPointer : public AssetPointer_Base
	{
	public:
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
	};
}