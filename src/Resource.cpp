#include "Resource.h"
#include "Renderer.h"

using namespace BlackMagic;

Resource::Resource() 
	: _renderer(nullptr),
	_resource(nullptr)
{}

Resource::Resource(Renderer* renderer, ResourceHandle handle)
	: _renderer(renderer),
	_resource(handle)
{}

Resource::Resource(const Resource& r)
{
	*this = r;
}

Resource& Resource::operator=(const Resource& r)
{
	_renderer = r._renderer;
	_resource = r._resource;
	if (_renderer)
		_renderer->AddResourceRef(_resource);
	return *this;
}

Resource::~Resource()
{
	if (_renderer && _resource)
		_renderer->ReleaseResource(_resource);
}