#pragma once
#pragma comment(lib, "assimp-vc140-mt.lib")

#include <DirectXCollision.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <string>

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

	DirectX::BoundingOrientedBox obb;
	DirectX::BoundingSphere sphere;
};

class MeshData : public ProxyHandler<Mesh, MeshData>
{
public:
	~MeshData();

	void Init(ID3D11Device* device);

	Handle Get(std::string modelPath);
	void Revoke(Handle handle);

	Handle LoadMesh(std::string modelPath);
	const std::string root = "assets/models/";

private:
	ID3D11Device* device;
};
