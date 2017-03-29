#pragma once

namespace proj
{
	struct defaults
	{
		char* meshPath;
	};

	struct meta
	{
		size_t numMeshes;
		size_t numScenes;
	};

	struct asset
	{
		char* path;
		char* name;
	};

	struct assets
	{
		asset* l_meshes;
		asset* l_scenes;
	};

	struct file
	{
		defaults defaultAssets;
		meta metaData;
		assets assetData;
		float camPos[3];
		float camRot[4];
	};
}

namespace scene
{
	struct transform
	{
		float pos[3];
		float scale;
		float rot[4];
	};

	struct entity
	{
		transform trans;
		size_t meshIndex;
	};

	struct file
	{
		size_t numEntities;
		entity* l_entities;
	};
}
