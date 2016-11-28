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
		GraphicsBuffer() {}
		GraphicsBuffer(void* ptr) : buffer(ptr) {}

		void* buffer;
	};
}