#pragma once

#include <new>

namespace BlackMagic
{
	typedef unsigned char byte;

	template<typename Alloc, typename T, typename... Args>
	inline T* AllocateAndConstruct(Alloc* allocator, int num, Args... args)
	{
		T* space = allocator->allocate<T>(num);
		if (space)
		{
			space = new (space) T(args...);
		}
		return space;
	}

	template<typename Alloc, typename T>
	inline T* AllocateAndConstruct(Alloc* allocator, int num)
	{
		T* space = allocator->allocate<T>(num);
		if (space)
		{
			space = new (space) T;
		}
		return space;
	}

	template<typename Alloc, typename T, typename... Args>
	inline void DestructAndDeallocate(Alloc* allocator, T* thing, int num)
	{
		for (int i = 0; i < num; i++)
		{
			thing[i].~T();
		}
		allocator->deallocate(thing, num);
	}

	constexpr size_t alignSize(size_t minAlign, size_t size)
	{
		return (size + (minAlign - 1)) & ~(minAlign - 1);;
	}

	template<typename T>
	constexpr size_t alignSize(size_t minAlign)
	{
		return alignSize(minAlign, sizeof(T));
	}
}