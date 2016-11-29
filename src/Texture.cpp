#include "Texture.h"

using namespace BlackMagic;

Texture::Texture(GraphicsTexture srView, GraphicsRenderTarget rtView)
	:
	_rtView(rtView),
	_srView(srView)
{
}

Texture::~Texture()
{
	/*if (_rtView)
	{
		_rtView->Release();
	}
	if (_srView)
	{
		_srView->Release();
	}*/
}

GraphicsTexture Texture::GetGraphicsTexture() const
{
	return _srView;
}

GraphicsRenderTarget Texture::GetGraphicsRenderTarget() const
{
	return _rtView;
}
