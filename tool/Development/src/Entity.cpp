#include "Entity.h"

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

Entity* EntityData::newEntity()
{
	if (entityCount == numEntities)
	{
		numEntities += 16;
		Entity* newEntities = new Entity[numEntities];

		memcpy_s(newEntities, numEntities, entities, numEntities - 16);

		delete[] entities;
		entities = newEntities;
	}

	Entity* entity = &entities[entityCount++];

	entity->transform = TransformData::instance().newTransform();

	return entity;
}

void EntityData::deleteEntity(Entity* entity)
{
	if (!entityCount)
		return;

	size_t index = entity - entities;

	TransformData::instance().deleteTransform(entity->transform);

	if (index != --entityCount)
	{
		entities[index] = entities[entityCount];
	}
}

size_t EntityData::count()
{
	return entityCount;
}

Entity* EntityData::operator[](size_t i)
{
	return entities + i;
}
