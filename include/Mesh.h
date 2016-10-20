#pragma once

#include <d3d11.h>
#include <string>
#include "Vertex.h"
#include "IResource.h"

class Mesh : public IResource
{
public:
	Mesh(const std::wstring& file, ID3D11Device* device);
	~Mesh();

	ID3D11Buffer* VertexBuffer() const;
	ID3D11Buffer* IndexBuffer() const;

	size_t IndexCount() const;

private:
	ID3D11Buffer *_vBuf, *_iBuf;
	size_t _numIndices;
};
