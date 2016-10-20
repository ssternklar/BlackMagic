#pragma once
#include<stddef.h>

typedef unsigned char byte;

namespace BlackMagic
{
	template<typename T, size_t alignment>
	class FixedBlockAllocator
	{
		static const int alignSize = sizeof(T) + ((alignment - 1) - (sizeof(T) % (alignment - 1)));

		struct FixedBlockTracking
		{
			FixedBlockTracking* next;
			byte padding[alignSize - sizeof(FixedBlockTracking*)];
		};

		union BlockSize_Union
		{
			T block;
			FixedBlockTracking tracking;
		};

		size_t numBlocks;
		BlockSize_Union* storage;
		FixedBlockTracking* next;

	public:
		FixedBlockAllocator(size_t size, byte* buffer = nullptr)
		{
			if (buffer == nullptr)
			{
				byte* ptr = reinterpret_cast<byte*>(this);
				size = size - alignSize;
				storage = (BlockSize_Union*)(ptr + alignSize);
			}
			else
			{
				storage = (BlockSize_Union*)buffer;
			}

			numBlocks = size / sizeof(BlockSize_Union);

			for (int i = 0; i < numBlocks - 1; i++)
			{
				storage[i].tracking.next = &(storage[i + 1].tracking);
			}
			storage[numBlocks - 1].tracking.next = nullptr;
			next = &(storage[0].tracking);
		}

		T* allocate()
		{
			T* ret = nullptr;
			if (next)
			{
				FixedBlockTracking* toUse = next;
				next = toUse->next;
				void* ptr = static_cast<void*>(toUse);
				ret = new (ptr) T();
			}
			return ret;
		}

		void deallocate(T* dealloc, bool callDestructor)
		{
			FixedBlockTracking* ptr = (FixedBlockTracking*)dealloc;
			int index = ptr - &(storage->tracking);
			if (&(storage[index].tracking) == ptr)
			{
				if (callDestructor)
				{
					dealloc->~T();
				}
				storage[index].tracking.next = next;
				next = &(storage[index].tracking);
			}
		}
	};
}