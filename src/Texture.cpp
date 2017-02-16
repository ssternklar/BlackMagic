#include "Texture.h"
#include "Renderer.h"

using namespace BlackMagic;

Texture::Texture(Renderer* device, GraphicsTexture srView, GraphicsRenderTarget rtView)
	: _rtView(rtView),
	_srView(srView),
	IResource(device)
{
}

Texture::~Texture()
{
	device->ReleaseTexture(_srView);
	device->ReleaseRenderTarget(_rtView);
}

GraphicsTexture Texture::GetGraphicsTexture() const
{
	return _srView;
}

GraphicsRenderTarget Texture::GetGraphicsRenderTarget() const
{
	return _rtView;
}
