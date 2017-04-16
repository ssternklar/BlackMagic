#include "Mesh.h"

#include <cstdio>
#include <fstream>
#include <vector>
#include "Vertex.h"
#include "Renderer.h"
#include "GraphicsTypes.h"

using namespace BlackMagic;
using namespace DirectX;
/*
void OrthonormalizeTB(Vertex& v, Vector3 tO, Vector3 uO)
{
	auto n = v.Normal;
	auto t = tO - Dot(tO, n)*n;
	auto u = uO - Dot(uO, n)*n - Dot(uO, t)*t;

	t = Normalize(t);
	u = Normalize(u);

	if (Dot(Cross(n, t), u) < 0.0f)
		t = t * -1.0f;

	v.Tangent = t;
	v.Binormal = u;
}

void CalculateTBN(Vertex& v1, Vertex& v2, Vertex& v3)
{
	auto p1 = v1.Position;
	auto p2 = v2.Position;
	auto p3 = v3.Position;

	auto n = v1.Normal;

	auto t1 = v1.UV;
	auto t2 = v2.UV;
	auto t3 = v3.UV;

	auto x = p2 - p1;
	auto y = p3 - p1;
	auto a = t2 - t1;
	auto b = t3 - t1;

	float det = 1 / (a.data[0] * b.data[1] - a.data[1] * b.data[0]);
	auto t = (x * b.data[1] - y * a.data[1])*det;
	auto u = (y * a.data[0] - x * b.data[0])*det;

	OrthonormalizeTB(v1, t, u);
	OrthonormalizeTB(v2, t, u);
	OrthonormalizeTB(v3, t, u);
}*/

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
