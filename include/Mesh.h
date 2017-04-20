#pragma once

#include <string>
#include "GraphicsTypes.h"
#include "allocators\globals.h"

namespace BlackMagic
{
	class Renderer;
	class Mesh : public Resource
	{
	public:
		Mesh();
		Mesh(byte* vertexData, int vertexCount, byte* indexData, int indexCount, Renderer* device);
		~Mesh();

		const Buffer& VertexBuffer();
		const Buffer& IndexBuffer();

		size_t IndexCount() const;
		void Set(const Buffer& vertexBuffer, const Buffer& indexBuffer, size_t numIndices);

	private:
		Buffer _vBuf, _iBuf;
		size_t _numIndices;
	};
}