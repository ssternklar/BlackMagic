#pragma once

#include "Patterns.h"
#include "Transform.h"
#include "Mesh.h"

struct Entity
{
	TransformData::Handle transform;
	MeshData::Handle mesh;
};

class EntityData : public Asset<Entity, EntityData>
{
public:
	EntityData() {};
	~EntityData() {};

	Handle Get();
	void Revoke(Handle entity);
	Entity* Entities();
};