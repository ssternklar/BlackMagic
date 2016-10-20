#pragma once
#include<stddef.h>

typedef unsigned char byte;

namespace BlackMagic
{

	class StackAllocator
	{	
	private:
		size_t size;
		size_t alignment;
		byte* storage;
		byte* currentPointer;

		template<typename T>
		inline size_t alignSize(size_t minAlign)
		{
			return sizeof(T) + ((minAlign - 1) - (sizeof(T) % (minAlign - 1)));
		}

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
			currentPointer = storage;
		}

		template<typename T>
		T* allocate()
		{
			T* ret = nullptr;
			size_t offset = alignSize<T>(alignment);
			if (currentPointer + offset <= storage + size)
			{
				ret = new (currentPointer) T();
				currentPointer += offset;
			}
			return ret;
		}
	
		template<typename T>
		void deallocate(T* dealloc, bool callDestructor)
		{
			if ((byte*)dealloc >= storage && (byte*)dealloc < currentPointer && sizeof(T) < currentPointer - storage)
			{
				size_t offset = alignSize<T>(alignment);
				if (callDestructor)
				{
					dealloc->~T();
				}
				currentPointer -= offset;
			}
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