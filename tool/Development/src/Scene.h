#pragma once

#include "Shaders.h"
#include "Mesh.h"
#include "Entity.h"

struct Scene
{
	ShaderData* shaders;
	MeshData* meshes;
	EntityData* entities;
};
