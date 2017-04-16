#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#include "Mesh.h"
#include "Assets.h"
#include "FileFormats.h"
#include "FileUtil.h"

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
}

MeshData::Handle MeshData::Get(std::string modelPath)
{
	std::string fullPath = root + modelPath;

	Handle h = AssetManager::Instance().GetHandle<MeshData>(fullPath);
	if (h.ptr())
		return h;

	h = Load(fullPath);

	if (!h.ptr())
		return h;

	AssetManager::Instance().TrackAsset<MeshData>(h, fullPath);

	return h;
}

MeshData::Handle MeshData::Load(std::string modelPath)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(modelPath,
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

	Handle h = ProxyHandler::Get();
	h->vertCount = 0;
	h->faceCount = 0;

	for (size_t i = 0; i < scene->mNumMeshes; ++i)
	{
		h->vertCount += scene->mMeshes[i]->mNumVertices;
		h->faceCount += scene->mMeshes[i]->mNumFaces * 3;
	}

	UINT vertOffset = 0, faceOffset = 0;
	h->verts = new Mesh::Vertex[h->vertCount];
	h->faces = new UINT[h->faceCount];

	for (size_t m = 0; m < scene->mNumMeshes; ++m)
	{
		aiMesh* mesh = scene->mMeshes[m];

		for (size_t i = 0; i < mesh->mNumVertices; ++i)
		{
			h->verts[vertOffset + i].position = DirectX::XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			h->verts[vertOffset + i].normal = DirectX::XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			h->verts[vertOffset + i].tangent = DirectX::XMFLOAT3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
			h->verts[vertOffset + i].bitangent = DirectX::XMFLOAT3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
			h->verts[vertOffset + i].uv = DirectX::XMFLOAT2(mesh->mTextureCoords[0][i].x, 1 - mesh->mTextureCoords[0][i].y);
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

	DirectX::BoundingOrientedBox::CreateFromPoints(h->obb, h->vertCount, &h->verts[0].position, sizeof(Mesh::Vertex));
	DirectX::BoundingSphere::CreateFromPoints(h->sphere, h->vertCount, &h->verts[0].position, sizeof(Mesh::Vertex));

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Mesh::Vertex) * h->vertCount;
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

	AssetManager::Instance().StopTrackingAsset<MeshData>(handle);
	ProxyHandler::Revoke(handle);
}

void MeshData::Export(std::string path, Handle handle)
{
	FileUtil::CreateDirectoryRecursive(path);

	FILE* meshFile;
	fopen_s(&meshFile, path.c_str(), "wb");
	if (!meshFile)
	{
		printf("Failed to write mesh file '%s'", path.c_str());
		return;
	}

	Export::Mesh::File fileData;

	// meta
	fileData.blockCount = 3;
	
	fileData.boundsMeta.offsetInBytes = sizeof(Export::Mesh::File::blockCount) + sizeof(Export::Mesh::Block) * fileData.blockCount;
	fileData.boundsMeta.sizeInBytes = sizeof(Export::Mesh::Bounds);
	fileData.boundsMeta.elementCount = 1;
	fileData.boundsMeta.elementSize = sizeof(Export::Mesh::Bounds);

	fileData.vertexMeta.offsetInBytes = fileData.boundsMeta.offsetInBytes + fileData.boundsMeta.sizeInBytes;
	fileData.vertexMeta.sizeInBytes = (uint16_t)(sizeof(Export::Mesh::Vertex) * handle->vertCount);
	fileData.vertexMeta.elementCount = (uint16_t)handle->vertCount;
	fileData.vertexMeta.elementSize = sizeof(Export::Mesh::Vertex);

	fileData.indexMeta.offsetInBytes = fileData.vertexMeta.offsetInBytes + fileData.vertexMeta.sizeInBytes;
	fileData.indexMeta.sizeInBytes = (uint16_t)(sizeof(uint32_t) * handle->faceCount);
	fileData.indexMeta.elementCount = (uint16_t)handle->faceCount;
	fileData.indexMeta.elementSize = sizeof(uint32_t);

	fwrite(&fileData.blockCount, sizeof(uint8_t), 1, meshFile);
	fwrite(&fileData.boundsMeta.offsetInBytes, sizeof(Export::Mesh::Block), fileData.blockCount, meshFile);

	// bounds
	Export::Mesh::Bounds bounds = {};
	memcpy_s(&bounds.obbCenter[0], sizeof(float) * 3, &handle->obb.Center.x, sizeof(DirectX::XMFLOAT3));
	memcpy_s(&bounds.halfSize[0], sizeof(float) * 3, &handle->obb.Extents.x, sizeof(DirectX::XMFLOAT3));
	memcpy_s(&bounds.sphere[0], sizeof(float) * 3, &handle->sphere.Center.x, sizeof(DirectX::XMFLOAT3));
	memcpy_s(&bounds.sphere[3], sizeof(float), &handle->sphere.Radius, sizeof(float));

	fwrite(&bounds.obbCenter[0], sizeof(Export::Mesh::Bounds), 1, meshFile);

	// vertices
	Export::Mesh::Vertex vert = {};
	size_t i;
	for (i = 0; i < handle->vertCount; ++i)
	{
		memcpy_s(&vert.position, sizeof(float) * 3, &handle->verts[i].position.x, sizeof(DirectX::XMFLOAT3));
		memcpy_s(&vert.normal, sizeof(float) * 3, &handle->verts[i].normal.x, sizeof(DirectX::XMFLOAT3));
		memcpy_s(&vert.tangent, sizeof(float) * 3, &handle->verts[i].tangent.x, sizeof(DirectX::XMFLOAT3));
		memcpy_s(&vert.biTangent, sizeof(float) * 3, &handle->verts[i].bitangent.x, sizeof(DirectX::XMFLOAT3));
		memcpy_s(&vert.uv, sizeof(float) * 2, &handle->verts[i].uv.x, sizeof(DirectX::XMFLOAT2));

		fwrite(&vert.position[0], sizeof(Export::Mesh::Vertex), 1, meshFile);
	}

	// write indexes
	fwrite(&handle->faces[0], fileData.indexMeta.elementSize, handle->faceCount, meshFile);

	fclose(meshFile);
}
