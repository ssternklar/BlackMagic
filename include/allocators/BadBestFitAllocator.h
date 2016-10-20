#pragma once
#include<stddef.h>
#include<new>

typedef unsigned char byte;

namespace BlackMagic
{
	class BestFitAllocator
	{
	private:

		template<typename T>
		inline size_t alignSize(size_t minAlign)
		{
			return sizeof(T) + ((minAlign - 1) - (sizeof(T) % (minAlign - 1)));
		}

		class header
		{
		public:
			size_t sizeOfThis;
			header* nextFree;
		};
		size_t size;
		size_t alignment;
		byte* storage;
		header* freeList;

	public:
		BestFitAllocator(size_t maxSize, size_t alignment, byte* buffer = nullptr)
		{
			this->alignment = alignment;
			byte* ptr = buffer;
			if (buffer == nullptr)
			{
				byte* ptr = reinterpret_cast<byte*>(this);
			}
			size_t offset = alignSize<BestFitAllocator>(alignment);
			size = maxSize - offset;
			storage = buffer + (offset);

			//fake construction of a header
			freeList = reinterpret_cast<header*>(storage);
			freeList->sizeOfThis = size;
			freeList->nextFree = nullptr;
		}

		template<typename T>
		T* allocate()
		{
			header* toAlloc = nullptr;
			header* prevToAlloc = nullptr;
			header* freeIter = freeList;
			header* prevFreeIter = nullptr;
			while (freeIter != nullptr)
			{
				if (freeIter->sizeOfThis > sizeof(T))
				{
					if (toAlloc)
					{
						if (toAlloc->sizeOfThis > freeIter->sizeOfThis)
						{
							prevToAlloc = prevFreeIter;
							toAlloc = freeIter;
						}
					}
					else
					{
						prevToAlloc = prevFreeIter;
						toAlloc = freeIter;
					}
				}
				prevFreeIter = freeIter;
				freeIter = freeIter->nextFree;
			}

			if (toAlloc)
			{
				size_t minSize = alignSize<T>(alignment);
				T* ret = nullptr;
				if (toAlloc->sizeOfThis - minSize > minSize)
				{
					//shift block right and set up
					header* movedToAlloc = reinterpret_cast<header*>(reinterpret_cast<byte*>(toAlloc) + minSize);
					movedToAlloc->nextFree = toAlloc->nextFree;
					movedToAlloc->sizeOfThis = toAlloc->sizeOfThis - minSize;
					prevToAlloc->nextFree = movedToAlloc;
					return new (toAlloc) T();
				}
				else
				{
					prevToAlloc->nextFree = toAlloc->nextFree;
					return new (toAlloc) T();
				}
			}

			return nullptr;
		}

	};
}