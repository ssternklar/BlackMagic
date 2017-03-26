#pragma once

#include "Patterns.h"
#include "Transform.h"
#include "Mesh.h"

struct Entity
{
	TransformData::Handle transform;
	MeshData::Handle mesh;
};

class EntityData : public ProxyHandler<Entity, EntityData>
{
public:
	EntityData() {};
	~EntityData() {};

	Handle Get();
	void Revoke(Handle entity);
	Entity* Entities();
};