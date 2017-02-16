#include "IResource.h"
#include "Renderer.h"

using namespace BlackMagic;

IResource::IResource() 
{
	device = nullptr;
}
IResource::IResource(Renderer* device) : device(device)
{
	this->device = device;
}