#pragma once

#include <d3d11.h>
#include <string>

#include "Patterns.h"

struct TextureDesc
{
	enum Type
	{
		FLAT,
		CUBEMAP
	};

	enum Usage : unsigned char
	{
		READ = 1 << 0,
		WRITE = 1 << 1
	};

	size_t height;
	size_t width;
	size_t depth;
	DXGI_FORMAT format;
	Type type;
	Usage usage;
	void* InitialData;
};

struct Texture
{
	ID3D11Resource* tex;
	ID3D11ShaderResourceView* srv;
	ID3D11RenderTargetView* rtv;
};

class TextureData : public ProxyHandler<Texture, TextureData>
{
public:
	~TextureData();

	void Init(ID3D11Device* device, ID3D11DeviceContext* context);

	Handle Create(TextureDesc desc);
	Handle CreateEmpty(D3D11_TEXTURE2D_DESC& desc);
	Handle Get(std::string texturePath);
	void Revoke(Handle handle, bool isEngine = false);

	void Export(std::string path, Handle handle);
	Handle Load(std::string path, TextureDesc::Type type, TextureDesc::Usage usage);
	const std::string root = "assets/textures/";

private:
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	size_t BytesPerPixel(DXGI_FORMAT fmt);
};
