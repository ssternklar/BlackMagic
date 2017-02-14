#pragma once
#include"globals.h"
#include<stddef.h>

namespace BlackMagic
{

	class StackAllocator
	{	
	private:
		size_t size;
		size_t alignment;
		byte* storage;
		byte* currentPointer;

	public:
		//If buffer is nullptr, assume that the buffer starts at "this"
		StackAllocator(size_t alignment, size_t maxSize, byte* buffer = nullptr)
		{
			size = maxSize;
			this->alignment = alignment;
			if(buffer == nullptr)
			{
				byte* ptr = reinterpret_cast<byte*>(this);
				size_t offset = alignSize<StackAllocator>(alignment);
				size = maxSize - offset;
				storage = ptr + offset;
			}
			else
			{
				storage = buffer;
			}

			byte alignmentIssues = (reinterpret_cast<size_t>(storage) & (alignment - 1));
			size -= alignmentIssues;
			storage += alignmentIssues;

			currentPointer = storage;
		}

		void* allocate(size_t size, size_t n = 1)
		{
			size_t requestedSize = size * n;
			void* ret = nullptr;
			size_t offset = alignSize(alignment, requestedSize);
			if (currentPointer + offset <= storage + this->size)
			{
				ret = currentPointer;
				currentPointer += offset;
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
			size_t requestedSize = size * n;
			if ((byte*)dealloc >= storage && (byte*)dealloc < currentPointer && requestedSize < (size_t)(currentPointer - storage))
			{
				size_t offset = alignSize(alignment, requestedSize);
				currentPointer -= offset;
			}
		}

		template<typename T>
		void deallocate(T* dealloc, size_t n = 1)
		{
			deallocate((void*)dealloc, sizeof(T), n);
		}

		size_t GetMaxSize()
		{
			return size;
		}

		size_t GetCurrentSize()
		{
			return currentPointer - storage;
		}

		size_t GetRemainingSize()
		{
			return size - (currentPointer - storage);
		}
	};

}