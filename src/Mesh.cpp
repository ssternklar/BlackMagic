#include "Mesh.h"

#include <cstdio>
#include <fstream>
#include <vector>
#include "Vertex.h"
#include "Renderer.h"
#include "GraphicsTypes.h"

using namespace BlackMagic;
using namespace DirectX;

void OrthonormalizeTB(Vertex& v, XMVECTOR tO, XMVECTOR uO)
{
	XMVECTOR n = XMLoadFloat3(&v.Normal);
	XMVECTOR t = tO - XMVector3Dot(tO, n)*n;
	XMVECTOR u = uO - XMVector3Dot(uO, n)*n - XMVector3Dot(uO, t)*t;

	XMVector3Normalize(t);
	XMVector3Normalize(u);

	if (XMVector3Dot(XMVector3Cross(n, t), u).m128_f32[0] < 0.0f)
		XMVectorNegate(t);


	XMFLOAT3 tangent, binormal;
	XMStoreFloat3(&tangent, t);
	XMStoreFloat3(&binormal, u);

	v.Tangent = tangent;
	v.Binormal = binormal;
}

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
	XMVECTOR t = (x * b.m128_f32[1] - y * a.m128_f32[1])*det;
	XMVECTOR u = (y * a.m128_f32[0] - x * b.m128_f32[0])*det;

	OrthonormalizeTB(v1, t, u);
	OrthonormalizeTB(v2, t, u);
	OrthonormalizeTB(v3, t, u);
}

Mesh::Mesh() : Resource(),
	_vBuf(nullptr, nullptr),
	_iBuf(nullptr, nullptr),
	_numIndices(0)
{}


Mesh::Mesh(BlackMagic::byte* vertexData, int vertexCount, BlackMagic::byte* indexData, int indexCount, Renderer* device) 
	: Resource()
{
	_numIndices = indexCount;
	_vBuf = device->CreateBuffer(Buffer::Type::VERTEX_BUFFER, vertexData, static_cast<UINT>(vertexCount * sizeof(Vertex)));
	_iBuf = device->CreateBuffer(Buffer::Type::INDEX_BUFFER, indexData, static_cast<UINT>(indexCount * sizeof(UINT)));
}


Mesh::~Mesh()
{
}

const Buffer& Mesh::VertexBuffer()
{
	return _vBuf;
}

const Buffer& Mesh::IndexBuffer()
{
	return _iBuf;
}

size_t Mesh::IndexCount() const
{
	return _numIndices;
}

void Mesh::Set(const Buffer& vertexBuffer, const Buffer& indexBuffer, size_t numIndices)
{
	_vBuf.~Buffer();
	_iBuf.~Buffer();
	_vBuf = vertexBuffer;
	_iBuf = indexBuffer;
	_numIndices = numIndices;
}
