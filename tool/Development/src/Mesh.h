#pragma once
#pragma comment(lib, "assimp-vc140-mt.lib")

#include <d3d11.h>
#include <DirectXMath.h>

#include "PointerProxy.h"

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

typedef proxy_ctr<Mesh>::proxy_ptr MeshHandle;

class MeshData
{
public:
	MeshData(ID3D11Device* device);
	~MeshData();

	MeshHandle NewMesh(char* modelPath);
	void DeleteMesh(MeshHandle handle);

private:
	ID3D11Device* device;

	ProxyVector<Mesh> proxy;
	MeshHandle defaultMesh;

	size_t numMeshes;
	size_t meshCount;
	Mesh* meshes;
};