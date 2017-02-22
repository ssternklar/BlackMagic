#include "Mesh.h"
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

MeshData* MeshData::ptr = nullptr;

void MeshData::Init(ID3D11Device* device)
{
	if (!ptr)
		ptr = new MeshData(device);
}

void MeshData::ShutDown()
{
	delete ptr;
}

MeshData::MeshData(ID3D11Device* device)
{
	numMeshes = 16;
	meshCount = 0;

	meshes = new Mesh[numMeshes];
	this->device = device;
	defaultMesh = newMesh("assets/models/teapot.obj");
}

MeshData::~MeshData()
{
	delete[] meshes;
}

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

	UINT vertCount = 0, faceCount = 0;

	for (size_t i = 0; i < scene->mNumMeshes; ++i)
	{
		vertCount += scene->mMeshes[i]->mNumVertices;
		faceCount += scene->mMeshes[i]->mNumFaces * 3;
	}

	//vec3 upper, lower;
	UINT vertOffset = 0, faceOffset = 0;
	Vertex* verts = new Vertex[vertCount];
	unsigned int* faces = new unsigned int[faceCount];

	//lower.x = std::min(lower.x, scene->mMeshes[0]->mVertices[0].x);
	//lower.y = std::min(lower.y, scene->mMeshes[0]->mVertices[0].y);
	//lower.z = std::min(lower.z, scene->mMeshes[0]->mVertices[0].z);
	//upper.x = std::max(upper.x, scene->mMeshes[0]->mVertices[0].x);
	//upper.y = std::max(upper.y, scene->mMeshes[0]->mVertices[0].y);
	//upper.z = std::max(upper.z, scene->mMeshes[0]->mVertices[0].z);

	for (size_t m = 0; m < scene->mNumMeshes; ++m)
	{
		aiMesh* mesh = scene->mMeshes[m];

		for (size_t i = 0; i < mesh->mNumVertices; ++i)
		{
			verts[vertOffset + i].Position = DirectX::XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			verts[vertOffset + i].Normal = DirectX::XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			verts[vertOffset + i].Tangent = DirectX::XMFLOAT3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
			verts[vertOffset + i].Bitangent = DirectX::XMFLOAT3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
			verts[vertOffset + i].UV = DirectX::XMFLOAT3(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y, mesh->mTextureCoords[0][i].z);

			//upper = vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			//lower = upper;
		}

		for (size_t i = 0; i < mesh->mNumFaces; ++i)
		{
			faces[faceOffset + i * 3] = vertOffset + mesh->mFaces[i].mIndices[0];
			faces[faceOffset + i * 3 + 1] = vertOffset + mesh->mFaces[i].mIndices[1];
			faces[faceOffset + i * 3 + 2] = vertOffset + mesh->mFaces[i].mIndices[2];
		}

		vertOffset += mesh->mNumVertices;
		faceOffset += mesh->mNumFaces * 3;
	}

	//bounds.halfSize = vec3(upper.x - lower.x, upper.y - lower.y, upper.z - lower.z);
	//bounds.halfSize /= 2.0f;
	//bounds.center.x = lower.x + bounds.halfSize.x;
	//bounds.center.y = lower.y + bounds.halfSize.y;
	//bounds.center.z = lower.z + bounds.halfSize.z;
	//
	//float dist = 0;
	//vec3 furthest;
	//for (size_t i = 0; i < vertCount; ++i)
	//{
	//	float distanceCheck = distanceSquared(vec3(verts[i].position.x, verts[i].position.y, verts[i].position.z), bounds.center);
	//	if (distanceCheck > dist)
	//	{
	//		furthest = vec3(verts[i].position.x, verts[i].position.y, verts[i].position.z);
	//		dist = distanceCheck;
	//	}
	//}
	//
	//dist = 0;
	//vec3 furthest2;
	//for (size_t i = 0; i < vertCount; ++i)
	//{
	//	float distanceCheck = distanceSquared(vec3(verts[i].position.x, verts[i].position.y, verts[i].position.z), furthest);
	//	if (distanceCheck > dist)
	//	{
	//		furthest2 = vec3(verts[i].position.x, verts[i].position.y, verts[i].position.z);
	//		dist = distanceCheck;
	//	}
	//}
	//
	//bounds.sphere = furthest2 - furthest;
	//bounds.radius = length(bounds.sphere) / 2.0f;
	//bounds.sphere = furthest + normalize(bounds.sphere) * bounds.radius;

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * vertCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialVertexData;
	initialVertexData.pSysMem = verts;

	device->CreateBuffer(&vbd, &initialVertexData, &meshes[meshCount].vertexBuffer);

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(unsigned int) * faceCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialIndexData;
	initialIndexData.pSysMem = faces;

	device->CreateBuffer(&ibd, &initialIndexData, &meshes[meshCount].indexBuffer);

	meshes[meshCount].indexCount = faceCount;

	delete[] verts;
	delete[] faces;

	return proxy.track(&meshes[meshCount++]);
}

void MeshData::deleteMesh(MeshHandle handle)
{
	if (!meshCount)
		return;

	size_t index = handle.ptr() - meshes;

	proxy.relinquish(handle.ptr());

	if (index != --meshCount)
	{
		proxy.move(meshes + meshCount, meshes + index);

		meshes[index] = meshes[meshCount];
	}
}

MeshHandle MeshData::getDefaultMesh()
{
	return defaultMesh;
}
