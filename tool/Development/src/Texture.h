#pragma once

#include <d3d11.h>
#include <string>

#include "Patterns.h"

struct Texture
{
	ID3D11Texture2D* tex;
	ID3D11ShaderResourceView* srView;
	ID3D11RenderTargetView* rtView;
};

class TextureData : public ProxyHandler<Texture, TextureData>
{
public:
	~TextureData();

	void Init(ID3D11Device* device);

	Handle Get(std::string texturePath);
	void Revoke(Handle handle);

	void Export(std::string path, Handle handle);
	Handle Load(std::string texturePath);
	const std::string root = "assets/textures/";

private:
	ID3D11Device* device;
};
