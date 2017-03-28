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
	};

	struct asset
	{
		char* path;
		char* name;
	};

	struct assets
	{
		asset* l_meshes;
	};

	struct file
	{
		defaults defaultAssets;
		meta metaData;
		assets assetData;
	};
}