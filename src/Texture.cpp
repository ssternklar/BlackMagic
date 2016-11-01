#include "Texture.h"

Texture::Texture(ID3D11Resource* tex, ID3D11ShaderResourceView* srView, ID3D11RenderTargetView* rtView)
	: _tex(tex),
	_rtView(rtView),
	_srView(srView)
{
}

Texture::~Texture()
{
	if (_rtView)
	{
		_rtView->Release();
	}
	if (_srView)
	{
		_srView->Release();
	}
	if (_tex)
	{
		_tex->Release();
	}
}

Texture::operator ID3D11ShaderResourceView*() const
{
	return _srView;
}

Texture::operator ID3D11RenderTargetView*() const
{
	return _rtView;
}
