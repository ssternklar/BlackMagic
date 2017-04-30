#pragma once

#include <stdint.h>

#include "Material.h"

// TODO clean swipe entire codebase for sizeof() usage against file formats and update them to not use types directly

namespace Internal
{
	namespace Proj
	{
		struct Meta
		{
			size_t nextUID;
			size_t defaultMeshUID;
			size_t defaultTextureUID;
			size_t defaultVertexShaderUID;
			size_t defaultPixelShaderUID;
			size_t defaultMaterialUID;
			size_t numMeshes;
			size_t numTextures;
			size_t numVertexShaders;
			size_t numPixelShaders;
			size_t numMaterials;
			size_t numScenes;
		};

		struct Asset
		{
			size_t uID;
			char* path;
			char* name;
		};

		struct Assets
		{
			Asset* l_meshes;
			Asset* l_textures;
			Asset* l_vertexShaders;
			Asset* l_pixelShaders;
			Asset* l_materials;
			Asset* l_scenes;
		};

		struct File
		{
			Meta metaData;
			Assets assetData;
			size_t* l_sceneIndices;
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
			uint16_t type;
			size_t meshIndex;
			size_t materialIndex;
		};

		struct File
		{
			uint8_t willExport;
			size_t numEntities;
			Entity* l_entities;
		};
	}

	namespace Material
	{
		struct File
		{
			size_t vertexShaderIndex;
			size_t PixelShaderIndex;
			size_t numTextures;
			size_t* l_texturesIndices;
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
			uint16_t filePathIndex;
			uint32_t fileSize;
		};

		struct File
		{
			uint16_t pathBlockSize;
			uint16_t numAssets;
			uint16_t numScenes;
			uint16_t* l_sceneUIDs;
			Asset* l_assets;
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
				float emptyScale[3];
		};

		struct Entity
		{
			Transform trans;
			uint16_t type;
			uint16_t meshUID;
			uint16_t materialUID;
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

	namespace Material
	{
		struct Shaders
		{
			uint16_t vertexShaderUID;
			uint16_t pixelShaderUID;
		};

		struct TextureResource
		{
			uint16_t nameIndex;
			uint16_t UID;
		};

		struct SamplerResource
		{
			uint16_t nameIndex;
		};

		struct File
		{
			Shaders shaders;
			uint8_t numTextures;
			uint8_t numSamplers;
			TextureResource* l_textures;
			SamplerResource* l_samplers;
			char* l_names;
		};
	}
}
