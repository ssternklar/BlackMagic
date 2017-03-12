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


Mesh::Mesh(const std::wstring& file, Renderer* device)
	: Resource()
{
	// File input object
	std::ifstream obj(file);

	// Check for successful open
	if (!obj.is_open())
		return;

	// Variables used while reading the file
	std::vector<XMFLOAT3> positions;     // Positions from the file
	std::vector<XMFLOAT3> normals;       // Normals from the file
	std::vector<XMFLOAT2> uvs;           // UVs from the file
	std::vector<Vertex> verts;           // Verts we're assembling
	std::vector<UINT> indices;           // Indices of these verts
	unsigned int vertCounter = 0;        // Count of vertices/indices
	char chars[100];                     // String for line reading

										 // Still good?
	while (obj.good())
	{
		// Get the line (100 characters should be more than enough)
		obj.getline(chars, 100);

		// Check the type of line
		if (chars[0] == 'v' && chars[1] == 'n')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			XMFLOAT3 norm;
			sscanf_s(
				chars,
				"vn %f %f %f",
				&norm.x, &norm.y, &norm.z);

			// Add to the list of normals
			normals.push_back(norm);
		}
		else if (chars[0] == 'v' && chars[1] == 't')
		{
			// Read the 2 numbers directly into an XMFLOAT2
			XMFLOAT2 uv;
			sscanf_s(
				chars,
				"vt %f %f",
				&uv.x, &uv.y);

			// Add to the list of uv's
			uvs.push_back(uv);
		}
		else if (chars[0] == 'v')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			XMFLOAT3 pos;
			sscanf_s(
				chars,
				"v %f %f %f",
				&pos.x, &pos.y, &pos.z);

			// Add to the positions
			positions.push_back(pos);
		}
		else if (chars[0] == 'f')
		{
			// Read the face indices into an array
			unsigned int i[12];
			int facesRead = sscanf_s(
				chars,
				"f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
				&i[0], &i[1], &i[2],
				&i[3], &i[4], &i[5],
				&i[6], &i[7], &i[8],
				&i[9], &i[10], &i[11]);

			// - Create the verts by looking up
			//    corresponding data from vectors
			// - OBJ File indices are 1-based, so
			//    they need to be adusted
			Vertex v1;
			v1.Position = positions[i[0] - 1];
			v1.UV = uvs[i[1] - 1];
			v1.Normal = normals[i[2] - 1];

			Vertex v2;
			v2.Position = positions[i[3] - 1];
			v2.UV = uvs[i[4] - 1];
			v2.Normal = normals[i[5] - 1];

			Vertex v3;
			v3.Position = positions[i[6] - 1];
			v3.UV = uvs[i[7] - 1];
			v3.Normal = normals[i[8] - 1];

			CalculateTBN(v1, v2, v3);

			// Flip the UV's since they're probably "upside down"
			v1.UV.y = 1.0f - v1.UV.y;
			v2.UV.y = 1.0f - v2.UV.y;
			v3.UV.y = 1.0f - v3.UV.y;

			// Add the verts to the vector
			verts.push_back(v1);
			verts.push_back(v2);
			verts.push_back(v3);

			// Add three more indices
			indices.push_back(vertCounter); vertCounter += 1;
			indices.push_back(vertCounter); vertCounter += 1;
			indices.push_back(vertCounter); vertCounter += 1;

			// Was there a 4th face?
			if (facesRead == 12)
			{
				// Make the last vertex
				Vertex v4;
				v4.Position = positions[i[9] - 1];
				v4.UV = uvs[i[10] - 1];
				v4.Normal = normals[i[11] - 1];

				CalculateTBN(v1, v3, v4);

				// Flip the y
				v4.UV.y = 1.0f - v4.UV.y;

				// Add a whole triangle
				verts.push_back(v1);
				verts.push_back(v3);
				verts.push_back(v4);

				// Add three more indices
				indices.push_back(vertCounter); vertCounter += 1;
				indices.push_back(vertCounter); vertCounter += 1;
				indices.push_back(vertCounter); vertCounter += 1;
			}
		}
	}

	// Close the file and create the actual buffers
	obj.close();

	// - At this point, "verts" is a vector of Vertex structs, and can be used
	//    directly to create a vertex buffer:  &verts[0] is the address of the first vert
	//
	// - The vector "indices" is similar. It's a vector of unsigned ints and
	//    can be used directly for the index buffer: &indices[0] is the address of the first int
	//
	// - "vertCounter" is BOTH the number of vertices and the number of indices
	// - Yes, the indices are a bit redundant here (one per vertex)

	_numIndices = indices.size();

	_vBuf = device->CreateBuffer(Buffer::Type::VERTEX_BUFFER, verts.data(), static_cast<UINT>(vertCounter * sizeof(Vertex)));
	_iBuf = device->CreateBuffer(Buffer::Type::INDEX_BUFFER, indices.data(), static_cast<UINT>(vertCounter * sizeof(UINT)));
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

