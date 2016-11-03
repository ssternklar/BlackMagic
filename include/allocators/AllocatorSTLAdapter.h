#pragma once

#include <type_traits>
#include "globals.h"

namespace BlackMagic
{
	template<typename T, typename Alloc>
	class AllocatorSTLAdapter
	{
	public:
		using value_type = T;
		using pointer = T*;

		Alloc* allocator;

		AllocatorSTLAdapter() : allocator(nullptr) {};
		AllocatorSTLAdapter(Alloc* alloc) : allocator(alloc) {};

		template<typename U>
		AllocatorSTLAdapter(AllocatorSTLAdapter<U, Alloc> const& other) : allocator(other.allocator) {};
		
		pointer allocate(std::size_t n)
		{
			if (allocator)
				return static_cast<pointer>(allocator->allocate(sizeof(T), n));
			return nullptr;
		}

		void deallocate(pointer p, std::size_t n)
		{
			if(allocator)
				allocator->deallocate(p, sizeof(T), n);
		}
		
		template<typename U>
		bool operator==(AllocatorSTLAdapter<U, Alloc> const& other) const
		{
			return allocator == other.allocator;
		}

		template<typename U>
		bool operator !=(AllocatorSTLAdapter<U, Alloc> const& other) const
		{
			return allocator != other.allocator;
		}

	};
}