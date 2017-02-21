#pragma once

#include "Transform.h"
#include "Mesh.h"

#include "PointerProxy.h"

struct Entity
{
	TransformHandle transform;
	MeshHandle mesh;
};

typedef proxy_ctr<Entity>::proxy_ptr EntityHandle;

class EntityData
{
public:
	static void Init();
	static EntityData* ptr;
	void ShutDown();

	EntityHandle newEntity();
	void deleteEntity(EntityHandle handle);

private:
	EntityData();
	~EntityData();
	EntityData(EntityData const&) = delete;
	void operator=(EntityData const&) = delete;

	ProxyVector<Entity> proxy;

	size_t numEntities;
	size_t entityCount;
	Entity* entities;
};