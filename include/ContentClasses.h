#pragma once

#include <PlatformResourceTypes.h>

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
			WAVFILE,
			SPLINE,
			SCENE,
			MATERIAL
		};
		char* resourceName;
		int uid;
		int size;
		void* resource;
		AtomicInt refcount = 0;
		ResourceType type;
	};

	struct UnknownContentType
	{
	};

	class AssetPointer_Base
	{
	public:
		ManifestEntry* entry = nullptr;
		AssetPointer_Base(ManifestEntry* entry)
		{
			if (entry)
			{
				this->entry = entry;
				BM_PLATFORM_ATOMIC_ADD(&(entry->refcount), 1);
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
				BM_PLATFORM_ATOMIC_ADD(&(entry->refcount), 1);
			}
		}

		AssetPointer_Base& operator=(const AssetPointer_Base& other)
		{
			if (other.entry)
			{
				entry = other.entry;
				BM_PLATFORM_ATOMIC_ADD(&(entry->refcount), 1);
			}
			return *this;
		}

		void reset()
		{
			if (entry)
			{
				BM_PLATFORM_ATOMIC_ADD(&(entry->refcount), -1);
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