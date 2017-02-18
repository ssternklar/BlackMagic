#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

struct Vertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT3 Tangent;
	DirectX::XMFLOAT3 Binormal;
	DirectX::XMFLOAT3 UV;
};

struct BoundingVectors
{
	DirectX::XMFLOAT3 halfSize;
	DirectX::XMFLOAT3 center;
	DirectX::XMFLOAT3 sphere;
	float radius;
};

struct MeshData
{

};

class Mesh
{
public:
private:
	ID3D11Buffer* vertexBuffer, *indexBuffer;
};