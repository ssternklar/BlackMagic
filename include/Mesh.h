#pragma once

#include <d3d11.h>
#include <string>
#include "IResource.h"

class Mesh : public IResource
{
public:
	Mesh();
	Mesh(const std::wstring& file, ID3D11Device* device);
	~Mesh();

	ID3D11Buffer* VertexBuffer() const;
	ID3D11Buffer* IndexBuffer() const;

	size_t IndexCount() const;
	void Set(ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer, size_t numIndices);

private:
	ID3D11Buffer *_vBuf, *_iBuf;
	size_t _numIndices;
};
