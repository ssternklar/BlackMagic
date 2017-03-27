#include "Texture.h"
#include "Renderer.h"

using namespace BlackMagic;

Texture::Texture(Renderer* renderer, ResourceHandle* tex, ShaderResource* srView, RenderTarget* rtView)
	: _rtView(rtView),
	_srView(srView),
	Resource(renderer, tex)
{}

Texture::Texture(const Texture& t)
{
	*this = t;
}

Texture::~Texture()
{
	_renderer->ReleaseResource(_srView);
	_renderer->ReleaseResource(_rtView);
}

Texture& Texture::operator=(const Texture& t)
{
	Resource::operator=(t);	
	_srView = t._srView;
	_rtView = t._rtView;
	_renderer->AddResourceRef(_srView);
	_renderer->AddResourceRef(_rtView);
	return *this;
}

ShaderResource* Texture::GetShaderResource() const
{
	return _srView;
}

RenderTarget* Texture::GetRenderTarget() const
{
	return _rtView;
}
