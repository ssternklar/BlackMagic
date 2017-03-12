#include "Mesh.h"
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

static float distanceSquared(float a[3], float b[3])
{
	float xx = a[0] - b[0];
	float yy = a[1] - b[1];
	float zz = a[2] - b[2];

	return xx*xx + yy*yy + zz*zz;
}

MeshData::MeshData(ID3D11Device* device)
{
	numMeshes = 16;
	meshCount = 0;

	meshes = new Mesh[numMeshes];
	this->device = device;
}

MeshData::~MeshData()
{
	for (size_t i = 0; i < meshCount; ++i)
	{
		meshes[i].vertexBuffer->Release();
		meshes[i].indexBuffer->Release();
		delete[] meshes[i].verts;
		delete[] meshes[i].faces;
	}

	delete[] meshes;
}

// update to use DirectXMath for the bounds
MeshHandle MeshData::newMesh(char* modelPath)
{
	if (meshCount == numMeshes)
	{
		numMeshes += 16;
		Mesh* newMeshes = new Mesh[numMeshes];

		memcpy_s(newMeshes, numMeshes, meshes, numMeshes - 16);
		proxy.move(meshes, newMeshes, numMeshes - 16);

		delete[] meshes;
		meshes = newMeshes;
	}

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

	meshes[meshCount].vertCount = 0;
	meshes[meshCount].faceCount = 0;

	for (size_t i = 0; i < scene->mNumMeshes; ++i)
	{
		meshes[meshCount].vertCount += scene->mMeshes[i]->mNumVertices;
		meshes[meshCount].faceCount += scene->mMeshes[i]->mNumFaces * 3;
	}

	UINT vertOffset = 0, faceOffset = 0;
	meshes[meshCount].verts = new Vertex[meshes[meshCount].vertCount];
	meshes[meshCount].faces = new UINT[meshes[meshCount].faceCount];

	float minX = scene->mMeshes[0]->mVertices[0].x;
	float minY = scene->mMeshes[0]->mVertices[0].y;
	float minZ = scene->mMeshes[0]->mVertices[0].z;
	float maxX = minX;
	float maxY = minY;
	float maxZ = minZ;

	for (size_t m = 0; m < scene->mNumMeshes; ++m)
	{
		aiMesh* mesh = scene->mMeshes[m];

		for (size_t i = 0; i < mesh->mNumVertices; ++i)
		{
			meshes[meshCount].verts[vertOffset + i].Position = DirectX::XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			meshes[meshCount].verts[vertOffset + i].Normal = DirectX::XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			meshes[meshCount].verts[vertOffset + i].Tangent = DirectX::XMFLOAT3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
			meshes[meshCount].verts[vertOffset + i].Bitangent = DirectX::XMFLOAT3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
			meshes[meshCount].verts[vertOffset + i].UV = DirectX::XMFLOAT3(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y, mesh->mTextureCoords[0][i].z);

			minX = minX < mesh->mVertices[i].x ? minX : mesh->mVertices[i].x;
			minY = minY < mesh->mVertices[i].y ? minY : mesh->mVertices[i].y;
			minZ = minZ < mesh->mVertices[i].z ? minZ : mesh->mVertices[i].z;
			maxX = maxX > mesh->mVertices[i].x ? maxX : mesh->mVertices[i].x;
			maxY = maxY > mesh->mVertices[i].y ? maxY : mesh->mVertices[i].y;
			maxZ = maxZ > mesh->mVertices[i].z ? maxZ : mesh->mVertices[i].z;
		}

		for (size_t i = 0; i < mesh->mNumFaces; ++i)
		{
			meshes[meshCount].faces[faceOffset + i * 3] = vertOffset + mesh->mFaces[i].mIndices[0];
			meshes[meshCount].faces[faceOffset + i * 3 + 1] = vertOffset + mesh->mFaces[i].mIndices[1];
			meshes[meshCount].faces[faceOffset + i * 3 + 2] = vertOffset + mesh->mFaces[i].mIndices[2];
		}

		vertOffset += mesh->mNumVertices;
		faceOffset += mesh->mNumFaces * 3;
	}

	meshes[meshCount].halfSize.x = (maxX - minX) * 0.5f;
	meshes[meshCount].halfSize.y = (maxY - minY) * 0.5f;
	meshes[meshCount].halfSize.z = (maxZ - minZ) * 0.5f;

	meshes[meshCount].center.x = minX + meshes[meshCount].halfSize.x;
	meshes[meshCount].center.y = minY + meshes[meshCount].halfSize.y;
	meshes[meshCount].center.z = minZ + meshes[meshCount].halfSize.z;

	//float dist = 0;
	//furthest[0] = bounds->aabbCenter[0];
	//furthest[1] = bounds->aabbCenter[1];
	//furthest[2] = bounds->aabbCenter[2];
	//
	//for (size_t m = 0; m < scene->mNumMeshes; ++m)
	//{
	//	aiMesh* mesh = scene->mMeshes[m];
	//	for (size_t j = 0; j < mesh->mNumVertices; ++j)
	//	{
	//		float distanceCheck = distanceSquared(mesh->mVertices[j].v, bounds->aabbCenter);
	//		if (distanceCheck > dist)
	//		{
	//			furthest[0] = mesh->mVertices[j].x;
	//			furthest[1] = mesh->mVertices[j].y;
	//			furthest[2] = mesh->mVertices[j].z;
	//			dist = distanceCheck;
	//		}
	//	}
	//}
	//
	//dist = 0;
	//furthest2[0] = furthest[0];
	//furthest2[1] = furthest[1];
	//furthest2[2] = furthest[2];
	//
	//for (size_t m = 0; m < scene->mNumMeshes; ++m)
	//{
	//	aiMesh* mesh = scene->mMeshes[m];
	//	for (size_t j = 0; j < mesh->mNumVertices; ++j)
	//	{
	//		float distanceCheck = distanceSquared(mesh->mVertices[j].v, furthest);
	//		if (distanceCheck > dist)
	//		{
	//			furthest2[0] = mesh->mVertices[j].x;
	//			furthest2[1] = mesh->mVertices[j].y;
	//			furthest2[2] = mesh->mVertices[j].z;
	//			dist = distanceCheck;
	//		}
	//	}
	//}
	//
	//bounds->sphere[0] = furthest2[0] - furthest[0];
	//bounds->sphere[1] = furthest2[1] - furthest[1];
	//bounds->sphere[2] = furthest2[2] - furthest[2];
	//float diameter = sqrtf(bounds->sphere[0] * bounds->sphere[0] + bounds->sphere[1] * bounds->sphere[1] + bounds->sphere[2] * bounds->sphere[2]);
	//bounds->sphere[3] = diameter * 0.5f;
	//bounds->sphere[0] = furthest[0] + (bounds->sphere[0] / diameter) * bounds->sphere[3];
	//bounds->sphere[1] = furthest[1] + (bounds->sphere[1] / diameter) * bounds->sphere[3];
	//bounds->sphere[2] = furthest[2] + (bounds->sphere[2] / diameter) * bounds->sphere[3];

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * meshes[meshCount].vertCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialVertexData;
	initialVertexData.pSysMem = meshes[meshCount].verts;

	device->CreateBuffer(&vbd, &initialVertexData, &meshes[meshCount].vertexBuffer);

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(unsigned int) * meshes[meshCount].faceCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialIndexData;
	initialIndexData.pSysMem = meshes[meshCount].faces;

	device->CreateBuffer(&ibd, &initialIndexData, &meshes[meshCount].indexBuffer);

	return proxy.track(&meshes[meshCount++]);
}

void MeshData::deleteMesh(MeshHandle handle)
{
	if (!meshCount)
		return;

	size_t index = handle.ptr() - meshes;

	proxy.relinquish(handle.ptr());

	handle->vertexBuffer->Release();
	handle->indexBuffer->Release();
	delete[] handle->verts;
	delete[] handle->faces;

	if (index != --meshCount)
	{
		proxy.move(meshes + meshCount, meshes + index);

		meshes[index] = meshes[meshCount];
	}
}
