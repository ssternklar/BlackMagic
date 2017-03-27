#include "Mesh.h"

#include <cstdio>
#include <fstream>
#include <vector>
#include "Vertex.h"
#include "Renderer.h"
#include "GraphicsTypes.h"

using namespace BlackMagic;
using namespace DirectX;

void CalculateTBN(Vertex& v1, Vertex& v2, Vertex& v3)
{
	XMVECTOR p1 = XMLoadFloat3(&v1.Position);
	XMVECTOR p2 = XMLoadFloat3(&v2.Position);
	XMVECTOR p3 = XMLoadFloat3(&v3.Position);

	XMVECTOR n = XMLoadFloat3(&v1.Normal);

	XMVECTOR t1 = XMLoadFloat2(&v1.UV);
	XMVECTOR t2 = XMLoadFloat2(&v2.UV);
	XMVECTOR t3 = XMLoadFloat2(&v3.UV);

	XMVECTOR x = p2 - p1;
	XMVECTOR y = p3 - p1;
	XMVECTOR a = t2 - t1;
	XMVECTOR b = t3 - t1;

	float det = 1 / (a.m128_f32[0] * b.m128_f32[1] - a.m128_f32[1] * b.m128_f32[0]);
	XMVECTOR t = (x * b.m128_f32[1] - y*a.m128_f32[1])*det;
	XMVECTOR u = (y*a.m128_f32[0] - x*b.m128_f32[0])*det;

	t = t - XMVector3Dot(t, n)*n;
	u = u - XMVector3Dot(u, n)*n - XMVector3Dot(u, t)*t;

	XMVector3Normalize(t);
	XMVector3Normalize(u);

	XMFLOAT3 tangent, binormal;
	XMStoreFloat3(&tangent, t);
	XMStoreFloat3(&binormal, u);

	v1.Tangent = v2.Tangent = v3.Tangent = tangent;
	v1.Binormal = v2.Binormal = v3.Binormal = binormal;
}

Mesh::Mesh() :
	IResource(nullptr),
	_vBuf(nullptr),
	_iBuf(nullptr),
	_numIndices(0)
{}


Mesh::Mesh(BlackMagic::byte* vertexData, int vertexCount, BlackMagic::byte* indexData, int indexCount, Renderer* device) : IResource(device)
{
	_numIndices = indexCount;
	_vBuf = device->CreateBuffer(GraphicsBuffer::BufferType::VERTEX_BUFFER, vertexData, static_cast<UINT>(vertexCount * sizeof(Vertex)));
	_iBuf = device->CreateBuffer(GraphicsBuffer::BufferType::INDEX_BUFFER, indexData, static_cast<UINT>(indexCount * sizeof(UINT)));
}


Mesh::~Mesh()
{
	device->CleanupBuffer(_vBuf);
	device->CleanupBuffer(_iBuf);
}

GraphicsBuffer Mesh::VertexBuffer() const
{
	return _vBuf;
}

GraphicsBuffer Mesh::IndexBuffer() const
{
	return _iBuf;
}

size_t Mesh::IndexCount() const
{
	return _numIndices;
}

void Mesh::Set(GraphicsBuffer vertexBuffer, GraphicsBuffer indexBuffer, size_t numIndices)
{
	device->CleanupBuffer(_vBuf);
	device->CleanupBuffer(_iBuf);
	_vBuf = vertexBuffer;
	_iBuf = indexBuffer;
	_numIndices = numIndices;
}

