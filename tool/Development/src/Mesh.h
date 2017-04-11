#pragma once
#pragma comment(lib, "assimp-vc140-mt.lib")

#include <DirectXCollision.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <string>

#include "Patterns.h"

struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 tangent;
	DirectX::XMFLOAT3 bitangent;
	DirectX::XMFLOAT2 uv;
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

	void Export(std::string path, Handle handle);
	Handle Load(std::string modelPath);
	const std::string root = "assets/models/";

private:
	ID3D11Device* device;
};
