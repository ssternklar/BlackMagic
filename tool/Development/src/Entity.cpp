#include "Entity.h"

EntityData::EntityData()
{
	numEntities = 4;
	entityCount = 0;

	entities = new Entity[numEntities];
}

EntityData::~EntityData()
{
	delete[] entities;
}

EntityHandle EntityData::NewEntity()
{
	if (entityCount == numEntities)
	{
		numEntities += 4;
		Entity* newEntities = new Entity[numEntities];

		memcpy_s(newEntities, sizeof(Entity) * numEntities, entities, sizeof(Entity) * (numEntities - 4));
		proxy.Move(entities, newEntities, numEntities - 4);

		delete[] entities;
		entities = newEntities;
	}

	Entity* entity = &entities[entityCount++];

	entity->transform = TransformData::Instance().NewTransform();

	return proxy.Track(entity);
}

void EntityData::DeleteEntity(EntityHandle handle)
{
	if (!entityCount)
		return;

	size_t index = handle.ptr() - entities;
	proxy.Relinquish(handle.ptr());
	TransformData::Instance().DeleteTransform(handle->transform);

	if (index != --entityCount)
	{
		proxy.Move(entities + entityCount, entities + index);
		entities[index] = entities[entityCount];
	}
}

size_t EntityData::Count()
{
	return entityCount;
}

Entity* EntityData::operator[](size_t i)
{
	return entities + i;
}
