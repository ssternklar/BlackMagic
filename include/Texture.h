#pragma once

#include <d3d11.h>
#include <memory>

#include "IResource.h"

class Texture : public IResource
{
public:
	Texture(ID3D11ShaderResourceView* view);
	~Texture();

	operator ID3D11ShaderResourceView*() const;

private:
	ID3D11ShaderResourceView* _view;
};