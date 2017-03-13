#pragma once
#pragma comment(lib, "assimp-vc140-mt.lib")

#include <d3d11.h>
#include <DirectXMath.h>
#include <map>

#include "Patterns.h"

struct Vertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT3 Tangent;
	DirectX::XMFLOAT3 Bitangent;
	DirectX::XMFLOAT3 UV;
};

struct Mesh
{
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	Vertex* verts;
	UINT* faces;
	size_t vertCount;
	size_t faceCount;

	DirectX::XMFLOAT4 sphere; // not implemented yet
	DirectX::XMFLOAT3 halfSize;
	DirectX::XMFLOAT3 center;
};

class MeshData : public Asset<Mesh, MeshData>
{
public:
	MeshData() {};
	~MeshData();

	void Init(ID3D11Device* device);

	Handle Get(std::string modelPath);
	void Revoke(Handle handle);

private:
	ID3D11Device* device;
	std::map<std::string, Handle> handles;
};