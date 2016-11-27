#pragma once
namespace BlackMagic
{
	struct GraphicsBuffer
	{
		enum BufferType
		{
			VERTEX_BUFFER,
			INDEX_BUFFER
		};

		void* buffer;

		template<class T>
		T GetBuffer<T>()
		{
			return reinterpret_cast<T>(buffer);
		}
	};
}