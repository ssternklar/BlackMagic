#pragma once

#include "PointerProxy.h"
#include "Transform.h"
#include "Mesh.h"

struct Entity
{
	TransformHandle transform;
	MeshHandle mesh;
};

typedef proxy_ctr<Entity>::proxy_ptr EntityHandle;

class EntityData
{
public:
	EntityData();
	~EntityData();

	EntityHandle NewEntity();
	void DeleteEntity(EntityHandle entity);
	size_t Count();
	Entity* operator[](size_t i);

private:
	ProxyVector<Entity> proxy;

	size_t numEntities;
	size_t entityCount;
	Entity* entities;
};