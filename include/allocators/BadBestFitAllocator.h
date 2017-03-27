#pragma once
#include"globals.h"
#include<stddef.h>
#include<new>

namespace BlackMagic
{
	class BestFitAllocator
	{
	private:
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
		BestFitAllocator() {}
		BestFitAllocator(size_t alignment, size_t maxSize, byte* buffer = nullptr)
		{
			size = maxSize;
			this->alignment = alignment;
			byte* ptr = buffer;
			if (buffer == nullptr)
			{
				buffer = reinterpret_cast<byte*>(this);
				size_t offset = alignSize<BestFitAllocator>(alignment);
				size = maxSize - offset;
				storage = buffer + (offset);
			}
			else
			{
				storage = buffer;
			}

			byte alignmentIssues = (reinterpret_cast<size_t>(storage) & (alignment - 1));
			size -= alignmentIssues;
			storage += alignmentIssues;

			freeList = reinterpret_cast<header*>(storage);
			freeList->sizeOfThis = size;
			freeList->nextFree = nullptr;
		}

		void* allocate(size_t size, size_t n = 1)
		{
			size_t requestedSize = size * n;
			header* toAlloc = nullptr;
			header* prevToAlloc = nullptr;
			header* freeIter = freeList;
			header* prevFreeIter = nullptr;
			while (freeIter != nullptr)
			{
				if (freeIter->sizeOfThis > requestedSize)
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
				size_t minSize = alignSize(alignment, requestedSize);
				if (toAlloc->sizeOfThis - minSize >= alignment)
				{
					header* movedToAlloc = reinterpret_cast<header*>(reinterpret_cast<byte*>(toAlloc) + minSize);
					movedToAlloc->nextFree = toAlloc->nextFree;
					movedToAlloc->sizeOfThis = toAlloc->sizeOfThis - minSize;
					if (prevToAlloc)
					{
						prevToAlloc->nextFree = movedToAlloc;
					}
					else
					{
						freeList = movedToAlloc;
					}
				}
				else
				{
					if(prevToAlloc)
					{
						prevToAlloc->nextFree = toAlloc->nextFree;
					}
					else
					{
						freeList = toAlloc->nextFree;
					}
				}

				return toAlloc;
			}

			return nullptr;
		}

		template<typename T>
		T* allocate(bool callConstructor = false, size_t n = 1)
		{
			void* memory = allocate(sizeof(T), n);
			if (callConstructor)
			{
				return new (memory) T[n];
			}
			return (T*)memory;
		}

		void deallocate(void* dealloc, size_t size, size_t n = 1)
		{
			size_t requestedSize = size * n;
			if (reinterpret_cast<byte*>(dealloc) < storage || reinterpret_cast<byte*>(dealloc) > storage + this->size)
			{
				throw "pointer to deallocate is not in this memory pool!";
			}
			size_t properSize = alignSize(alignment, requestedSize);

			header* openHeader = reinterpret_cast<header*>(dealloc);
			openHeader->sizeOfThis = properSize;
			header* prevHeader = nullptr;
			header* nextHeader = freeList;

			if (openHeader < freeList)
			{
				openHeader->nextFree = freeList;
				freeList = openHeader;
				nextHeader = openHeader->nextFree;
			}
			else
			{
				while (nextHeader < openHeader && nextHeader != nullptr)
				{
					prevHeader = nextHeader;
					nextHeader = nextHeader->nextFree;
				}
				prevHeader->nextFree = openHeader;
				openHeader->nextFree = nextHeader;
			}

			if (prevHeader && reinterpret_cast<byte*>(prevHeader) + prevHeader->sizeOfThis == (byte*)openHeader)
			{
				prevHeader->nextFree = openHeader->nextFree;
				prevHeader->sizeOfThis += openHeader->sizeOfThis;
				openHeader = prevHeader;
			}

			if (nextHeader && reinterpret_cast<byte*>(openHeader) + openHeader->sizeOfThis == (byte*)nextHeader)
			{
				openHeader->nextFree = nextHeader->nextFree;
				openHeader->sizeOfThis += nextHeader->sizeOfThis;
				nextHeader = openHeader;
			}
		}

		template<typename T>
		void deallocate(T* dealloc, bool callDestructor = false, size_t n = 1)
		{
			if (callDestructor)
			{
				for (int i = 0; i < n; i++)
				{
					dealloc[i].~T();
				}
			}
			deallocate((void*)dealloc, sizeof(T), n);
		}
		
		size_t FreeListSize()
		{
			size_t count = 0;
			header* list = freeList;
			while (list != nullptr)
			{
				list = list->nextFree;
				count++;
			}
			return count;
		}
	};
}