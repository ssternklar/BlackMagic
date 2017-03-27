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
		size_t* l_meshPathIndexes;
	};

	struct paths
	{
		char* l_meshPaths;
	};
}