#pragma once

namespace BlackMagic
{
	typedef unsigned char byte;

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