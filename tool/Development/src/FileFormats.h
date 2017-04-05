#pragma once

#include <stdint.h>

namespace Internal
{
	namespace Proj
	{
		struct Defaults
		{
			char* meshPath;
		};

		struct Meta
		{
			size_t numMeshes;
			size_t numScenes;
		};

		struct Asset
		{
			char* path;
			char* name;
		};

		struct Assets
		{
			Asset* l_meshes;
			Asset* l_scenes;
		};

		struct File
		{
			Defaults defaultAssets;
			Meta metaData;
			Assets assetData;
			float camPos[3];
			float camRot[4];
		};
	}

	namespace Scene
	{
		struct Transform
		{
			float pos[3];
			float scale;
			float rot[4];
		};

		struct Entity
		{
			Transform trans;
			size_t meshIndex;
		};

		struct File
		{
			size_t numEntities;
			Entity* l_entities;
		};
	}
}

namespace Export
{
	namespace Manifest
	{
		struct Asset
		{
			uint16_t uID;
			uint16_t fileSize;
			uint16_t filePathIndex;
		};

		struct File
		{
			uint16_t pathBlockSize;
			uint16_t numAssets;
			Asset* l_assets; // scenes are assets too
			char* l_filePaths;
		};
	}

	namespace Scene
	{
		struct Transform
		{
			float pos[3];
				float emptyPos;
			float rot[4];
			float scale;
		};

		struct Entity
		{
			Transform trans;
			uint16_t meshUID;
		};

		struct File
		{
			uint16_t numAssets;
			uint16_t* l_UIDs;
			size_t numEntities;
			Entity* l_entities;
		};
	}

	namespace Mesh
	{
		struct Vertex
		{
			float position[3];
				float emptyPos;
			float normal[3];
				float emptyNorm;
			float tangent[3];
				float emptyTang;
			float biTangent[3];
				float emptyBiTang;
			float uv[2]; // flipped on V
				float emptyUV[2];
		};

		struct Bounds
		{
			float obbCenter[3];
				float emptyCenter;
			float halfSize[3];
				float emptySize;
			float sphere[4];
		};

		struct Block
		{
			uint16_t offsetInBytes;
			uint16_t sizeInBytes;
			uint16_t elementCount;
			uint16_t elementSize;
		};

		struct File
		{
			uint8_t blockCount;
			Block boundsMeta;
			Block vertexMeta;
			Block indexMeta;
			Bounds bounds;
			Vertex* l_vertices;
			uint32_t* l_indices;
		};
	}
}
