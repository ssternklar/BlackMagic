#include "IResource.h"
#include "GraphicsDevice.h"

using namespace BlackMagic;

IResource::IResource() 
{
	device = nullptr;
}
IResource::IResource(GraphicsDevice* device) : device(device)
{
	this->device = device;
}