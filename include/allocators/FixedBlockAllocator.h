#pragma once
#include"globals.h"
#include<stddef.h>

namespace BlackMagic
{
	class FixedBlockAllocator
	{
	private:
		struct FixedBlockTracking
		{
			FixedBlockTracking* next;
		};

		size_t numBlocks;
		size_t blockSize;
		FixedBlockTracking* storage;
		FixedBlockTracking* next;

		FixedBlockTracking* indexBlock(size_t index)
		{
			return (storage + ((index * blockSize) / sizeof(FixedBlockTracking)));
		}

	public:
		FixedBlockAllocator(size_t alignment, size_t size, size_t blockSize, byte* buffer = nullptr)
		{
			size_t alignedSize = alignSize(alignment, blockSize);
			if (buffer == nullptr)
			{
				byte* ptr = reinterpret_cast<byte*>(this);
				size = size - alignedSize;
				storage = (FixedBlockTracking*)(ptr + alignedSize);
			}
			else
			{
				storage = (FixedBlockTracking*)buffer;
			}

			byte alignmentIssues = (byte)(reinterpret_cast<size_t>(storage) & (alignment - 1));
			size -= alignmentIssues;
			storage = reinterpret_cast<FixedBlockTracking*>(reinterpret_cast<byte*>(storage) + alignmentIssues);

			this->blockSize = alignedSize;
			numBlocks = size / alignedSize;

			for (int i = 0; i < numBlocks - 1; i++)
			{
				indexBlock(i)->next = indexBlock(i+1);
			}
			indexBlock(numBlocks - 1)->next = nullptr;
			next = storage;
		}

		void* allocate(size_t size, size_t n = 1)
		{
			if (size * n > blockSize)
			{
				assert(false); // "Requested allocation larger than block size!";
			}
			void* ret = nullptr;
			if (next)
			{
				FixedBlockTracking* toUse = next;
				next = toUse->next;
				ret = static_cast<void*>(toUse);
			}
			return ret;
		}

		template<typename T>
		T* allocate(size_t n = 1)
		{
			void* memory = allocate(sizeof(T), n);
			return (T*)memory;
		}

		void deallocate(void* dealloc, size_t size, size_t n = 1)
		{
			if (size * n > blockSize)
			{
				assert(false);// "Requested deallocation larger than block size!";
			}
			FixedBlockTracking* ptr = (FixedBlockTracking*)dealloc;
			size_t index = ((ptr - storage) * sizeof(FixedBlockTracking) / blockSize);
			if (indexBlock(index) == ptr)
			{
				indexBlock(index)->next = next;
				next = indexBlock(index);
			}
		}

		template<typename T>
		void deallocate(T* dealloc, size_t n = 1)
		{
			deallocate((void*)dealloc, sizeof(T), n);
		}
	};
}