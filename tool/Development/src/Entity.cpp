#include "Entity.h"
#include "Assets.h"

EntityData::Handle EntityData::Get()
{
	Handle h = ProxyHandler::Get();

	h->transform = TransformData::Instance().Get();
	h->mesh = AssetManager::Instance().defaults;
	h->material = AssetManager::Instance().defaults;

	return h;
}

void EntityData::Revoke(Handle handle)
{
	TransformData::Instance().Revoke(handle->transform);
	ProxyHandler::Revoke(handle);
}
