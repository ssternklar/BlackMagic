#include "Texture.h"
#include "GraphicsDevice.h"

using namespace BlackMagic;

Texture::Texture(GraphicsDevice* device, GraphicsTexture srView, GraphicsRenderTarget rtView)
	:
	IResource(device),
	_rtView(rtView),
	_srView(srView)
{
}

Texture::~Texture()
{
	device->CleanupTexture(_srView);
	device->CleanupRenderTarget(_rtView);
}

GraphicsTexture Texture::GetGraphicsTexture() const
{
	return _srView;
}

GraphicsRenderTarget Texture::GetGraphicsRenderTarget() const
{
	return _rtView;
}
