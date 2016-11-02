#pragma once

#include <d3d11.h>
#include <memory>

#include "IResource.h"

class Texture : public IResource
{
public:
	explicit Texture(ID3D11Resource* tex, ID3D11ShaderResourceView* srView, ID3D11RenderTargetView* rtView);
	~Texture();

	operator ID3D11ShaderResourceView*() const;
	operator ID3D11RenderTargetView*() const;

private:
	ID3D11Resource* _tex;
	ID3D11ShaderResourceView* _srView;
	ID3D11RenderTargetView* _rtView;
};