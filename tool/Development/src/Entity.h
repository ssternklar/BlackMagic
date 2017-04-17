#pragma once

#include "Patterns.h"
#include "Transform.h"
#include "Mesh.h"
#include "Material.h"

struct Entity
{
	TransformData::Handle transform;
	MeshData::Handle mesh;
	MaterialData::Handle material;
};

class EntityData : public ProxyHandler<Entity, EntityData>
{
public:
	Handle Get();
	void Revoke(Handle entity);
};