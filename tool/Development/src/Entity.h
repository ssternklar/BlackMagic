#pragma once

#include "Transform.h"
#include "Mesh.h"

struct Entity
{
	TransformHandle transform;
	MeshHandle mesh;
};

class EntityData
{
public:
	EntityData();
	~EntityData();

	Entity* newEntity();
	void deleteEntity(Entity* entity);
	size_t count();
	Entity* operator[](size_t i);

private:
	size_t numEntities;
	size_t entityCount;
	Entity* entities;
};