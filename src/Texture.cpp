#include "Texture.h"

Texture::Texture(ID3D11ShaderResourceView* view)
	: _view(view)
{}

Texture::~Texture()
{
	_view->Release();
}

Texture::operator ID3D11ShaderResourceView*() const
{
	return _view;
}

