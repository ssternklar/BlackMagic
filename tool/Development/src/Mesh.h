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
	size_t indexCount;

	DirectX::XMFLOAT4 sphere;
	DirectX::XMFLOAT3 halfSize;
	DirectX::XMFLOAT3 center;
};

typedef proxy_ctr<Mesh>::proxy_ptr MeshHandle;

class MeshData
{
public:
	static void Init(ID3D11Device* device);
	static MeshData* ptr;
	void ShutDown();

	MeshHandle newMesh(char* modelPath);
	void deleteMesh(MeshHandle handle);
	MeshHandle getDefaultMesh();

private:
	MeshData(ID3D11Device* device);
	~MeshData();
	MeshData(MeshData const&) = delete;
	void operator=(MeshData const&) = delete;

	ID3D11Device* device;

	ProxyVector<Mesh> proxy;
	MeshHandle defaultMesh;

	size_t numMeshes;
	size_t meshCount;
	Mesh* meshes;
};