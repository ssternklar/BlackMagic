#include "Entity.h"

EntityData* EntityData::ptr = nullptr;

void EntityData::Init()
{
	if (!ptr)
		ptr = new EntityData();
}

void EntityData::ShutDown()
{
	delete ptr;
}

EntityData::EntityData()
{
	numEntities = 16;
	entityCount = 0;

	entities = new Entity[numEntities];
}

EntityData::~EntityData()
{
	delete[] entities;
}

EntityHandle EntityData::newEntity()
{
	if (entityCount == numEntities)
	{
		numEntities += 16;
		Entity* newEntities = new Entity[numEntities];

		memcpy_s(newEntities, numEntities, entities, numEntities - 16);
		proxy.move(entities, newEntities, numEntities - 16);

		delete[] entities;
		entities = newEntities;
	}

	Entity* entity = &entities[entityCount++];

	entity->transform = TransformData::ptr->newTransform();
	entity->mesh = MeshData::ptr->getDefaultMesh();

	return proxy.track(entity);
}

void EntityData::deleteEntity(EntityHandle handle)
{
	if (!entityCount)
		return;

	size_t index = handle.ptr() - entities;

	proxy.relinquish(handle.ptr());

	if (index != --entityCount)
	{
		proxy.move(entities + entityCount, entities + index);

		entities[index] = entities[entityCount];
	}
}
