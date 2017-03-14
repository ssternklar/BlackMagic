#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#include "Mesh.h"

static float distanceSquared(float a[3], float b[3])
{
	float xx = a[0] - b[0];
	float yy = a[1] - b[1];
	float zz = a[2] - b[2];

	return xx*xx + yy*yy + zz*zz;
}

MeshData::~MeshData()
{
	for (size_t i = 0; i < size; ++i)
	{
		data[i].vertexBuffer->Release();
		data[i].indexBuffer->Release();
		delete[] data[i].verts;
		delete[] data[i].faces;
	}
}

void MeshData::Init(ID3D11Device* device)
{
	this->device = device;
	Get("teapot.obj");
}

MeshData::Handle MeshData::Get(std::string modelPath)
{
	auto check = handles.find(modelPath);
	if (check != handles.end())
		return check->second;

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(root + modelPath,
		aiProcess_CalcTangentSpace |
		aiProcess_JoinIdenticalVertices |
		aiProcess_FindInvalidData |
		aiProcess_GenNormals |
		aiProcess_ImproveCacheLocality |
		aiProcess_Triangulate |
		aiProcess_SortByPType |
		aiProcess_FixInfacingNormals |
		aiProcess_OptimizeMeshes |
		aiProcess_OptimizeGraph);

	if (!scene)
	{
		Handle e;
		return e;
	}

	Handle h = Asset::Get();
	handles[modelPath] = h;
	filePaths.push_back(modelPath);
	h->path = modelPath;

	h->vertCount = 0;
	h->faceCount = 0;

	for (size_t i = 0; i < scene->mNumMeshes; ++i)
	{
		h->vertCount += scene->mMeshes[i]->mNumVertices;
		h->faceCount += scene->mMeshes[i]->mNumFaces * 3;
	}

	UINT vertOffset = 0, faceOffset = 0;
	h->verts = new Vertex[h->vertCount];
	h->faces = new UINT[h->faceCount];

	for (size_t m = 0; m < scene->mNumMeshes; ++m)
	{
		aiMesh* mesh = scene->mMeshes[m];

		for (size_t i = 0; i < mesh->mNumVertices; ++i)
		{
			h->verts[vertOffset + i].Position = DirectX::XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			h->verts[vertOffset + i].Normal = DirectX::XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			h->verts[vertOffset + i].Tangent = DirectX::XMFLOAT3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
			h->verts[vertOffset + i].Bitangent = DirectX::XMFLOAT3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
			h->verts[vertOffset + i].UV = DirectX::XMFLOAT3(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y, mesh->mTextureCoords[0][i].z);
		}

		for (size_t i = 0; i < mesh->mNumFaces; ++i)
		{
			h->faces[faceOffset + i * 3] = vertOffset + mesh->mFaces[i].mIndices[0];
			h->faces[faceOffset + i * 3 + 1] = vertOffset + mesh->mFaces[i].mIndices[1];
			h->faces[faceOffset + i * 3 + 2] = vertOffset + mesh->mFaces[i].mIndices[2];
		}

		vertOffset += mesh->mNumVertices;
		faceOffset += mesh->mNumFaces * 3;
	}

	DirectX::BoundingOrientedBox::CreateFromPoints(h->obb, h->vertCount, &h->verts[0].Position, sizeof(Vertex));
	DirectX::BoundingSphere::CreateFromPoints(h->sphere, h->vertCount, &h->verts[0].Position, sizeof(Vertex));

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * h->vertCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialVertexData;
	initialVertexData.pSysMem = h->verts;

	device->CreateBuffer(&vbd, &initialVertexData, &h->vertexBuffer);

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(unsigned int) * h->faceCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialIndexData;
	initialIndexData.pSysMem = h->faces;

	device->CreateBuffer(&ibd, &initialIndexData, &h->indexBuffer);

	return h;
}

void MeshData::Revoke(Handle handle)
{
	handle->vertexBuffer->Release();
	handle->indexBuffer->Release();
	delete[] handle->verts;
	delete[] handle->faces;

	handles.erase(handles.find(handle->path));
	filePaths.erase(std::remove(filePaths.begin(), filePaths.end(), handle->path));

	Asset::Revoke(handle);
}
