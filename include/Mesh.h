#pragma once

#include <string>
#include "IResource.h"
#include "GraphicsTypes.h"
namespace BlackMagic
{
	class Renderer;
	class Mesh : public IResource
	{
	public:
		Mesh();
		Mesh(byte* vertexData, int vertexCount, byte* indexData, int indexCount, Renderer* device);
		~Mesh();

		GraphicsBuffer VertexBuffer() const;
		GraphicsBuffer IndexBuffer() const;

		size_t IndexCount() const;
		void Set(GraphicsBuffer vertexBuffer, GraphicsBuffer indexBuffer, size_t numIndices);

	private:
		GraphicsBuffer _vBuf, _iBuf;
		size_t _numIndices;
	};
}