#pragma once

#include <string>
#include "GraphicsTypes.h"

namespace BlackMagic
{
	class Renderer;
	class Mesh : public Resource
	{
	public:
		Mesh();
		Mesh(const std::wstring& file, Renderer* device);
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