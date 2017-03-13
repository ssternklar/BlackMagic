#include "Entity.h"

EntityData::Handle EntityData::Get()
{
	Handle h = Asset::Get();

	h->transform = TransformData::Instance().Get();
	h->mesh = MeshData::Instance().Get("teapot.obj");

	return h;
}

void EntityData::Revoke(Handle handle)
{
	TransformData::Instance().Revoke(handle->transform);
	Asset::Revoke(handle);
}

Entity* EntityData::Entities()
{
	return data;
}
