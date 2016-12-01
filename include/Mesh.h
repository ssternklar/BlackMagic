#pragma once

#include <string>
#include "IResource.h"
#include "GraphicsTypes.h"
namespace BlackMagic
{
	class GraphicsDevice;
	class Mesh : public IResource
	{
	public:
		Mesh();
		Mesh(const std::wstring& file, GraphicsDevice* device);
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