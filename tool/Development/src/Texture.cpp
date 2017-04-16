#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>

#include "Texture.h"
#include "Assets.h"
#include "FileUtil.h"
#include "StringManip.h"

TextureData::~TextureData()
{
	for (size_t i = 0; i < size; ++i)
	{
		DX_RELEASE(data[i].tex);
		DX_RELEASE(data[i].srv);
		DX_RELEASE(data[i].rtv);
	}
}

void TextureData::Init(ID3D11Device* device, ID3D11DeviceContext* context)
{
	this->device = device;
	this->context = context;
}

TextureData::Handle TextureData::Get(std::string texturePath)
{
	std::string fullPath = root + texturePath;

	Handle h = AssetManager::Instance().GetHandle<TextureData>(fullPath);
	if (h.ptr())
		return h;

	h = Load(fullPath, TextureDesc::Type::FLAT, TextureDesc::Usage::READ);

	if (!h.ptr())
		return h;

	AssetManager::Instance().TrackAsset<TextureData>(h, fullPath);

	return h;
}

TextureData::Handle TextureData::Load(std::string path, TextureDesc::Type type, TextureDesc::Usage usage)
{
	HRESULT result = S_OK;

	ID3D11Resource* tex = nullptr;
	ID3D11ShaderResourceView* srv = nullptr;
	ID3D11RenderTargetView* rtv = nullptr;

	wstring wpath = StringManip::utf8_decode(path);

	switch (type)
	{
	case TextureDesc::Type::FLAT:
		result = DirectX::CreateWICTextureFromFile(device, wpath.c_str(), &tex, &srv);
		break;

	case TextureDesc::Type::CUBEMAP:
		result = DirectX::CreateDDSTextureFromFile(device, wpath.c_str(), &tex, &srv);
		break;
	}

	if (result != S_OK)
	{
		Handle e;
		return e;
	}

	if (!(usage & TextureDesc::Usage::READ))
	{
		srv->Release();
		srv = nullptr;
	}

	if (usage & TextureDesc::Usage::WRITE)
	{
		result = device->CreateRenderTargetView(tex, nullptr, &rtv);

		if (result != S_OK)
		{
			tex->Release();
			srv->Release();
			Handle e;
			return e;
		}
	}

	Handle h = ProxyHandler::Get();

	h->tex = tex;
	h->srv = srv;
	h->rtv = rtv;

	return h;
}

TextureData::Handle TextureData::Create(TextureDesc desc)
{
	HRESULT result = S_OK;

	unsigned int bpp = BytesPerPixel(desc.format);

	D3D11_TEXTURE2D_DESC d3dDesc = { 0 };
	d3dDesc.Height = desc.height;
	d3dDesc.Width = desc.width;
	d3dDesc.BindFlags = (desc.usage & TextureDesc::Usage::READ ? D3D11_BIND_SHADER_RESOURCE : 0)
		| (desc.usage & TextureDesc::Usage::WRITE ? D3D11_BIND_RENDER_TARGET : 0);
	d3dDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dDesc.ArraySize = (desc.type == TextureDesc::CUBEMAP ? 6 : 1);
	d3dDesc.Format = desc.format;
	d3dDesc.MiscFlags = (desc.type == TextureDesc::CUBEMAP ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0)
		| (d3dDesc.BindFlags == (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET) ?
			D3D11_RESOURCE_MISC_GENERATE_MIPS : 0);
	d3dDesc.SampleDesc.Count = 1;
	d3dDesc.SampleDesc.Quality = 0;

	D3D11_SUBRESOURCE_DATA subDat = { 0 };
	subDat.pSysMem = desc.InitialData;
	subDat.SysMemPitch = bpp * desc.width;

	ID3D11Texture2D* tex = nullptr;
	ID3D11ShaderResourceView* srv = nullptr;
	ID3D11RenderTargetView* rtv = nullptr;

	result = device->CreateTexture2D(&d3dDesc, &subDat, &tex);

	if (result != S_OK)
	{
		Handle e;
		return e;
	}

	if (desc.usage & TextureDesc::Usage::READ)
	{
		result = device->CreateShaderResourceView(tex, nullptr, &srv);

		if (result != S_OK)
		{
			tex->Release();
			Handle e;
			return e;
		}

		if (d3dDesc.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS)
			context->GenerateMips(srv);
	}
	if (desc.usage & TextureDesc::Usage::WRITE)
	{
		device->CreateRenderTargetView(tex, nullptr, &rtv);

		if (result != S_OK)
		{
			tex->Release();
			srv->Release();
			Handle e;
			return e;
		}
	}

	Handle h = ProxyHandler::Get();

	h->tex = tex;
	h->srv = srv;
	h->rtv = rtv;

	return h;
}

void TextureData::Revoke(Handle handle)
{
	DX_RELEASE(handle->tex);
	DX_RELEASE(handle->srv);
	DX_RELEASE(handle->rtv);

	AssetManager::Instance().StopTrackingAsset<TextureData>(handle);
	ProxyHandler::Revoke(handle);
}

void TextureData::Export(std::string path, Handle handle)
{
	FileUtil::CopyFileViaPaths(AssetManager::Instance().GetAsset<TextureData>(handle).path, path);
}

size_t TextureData::BytesPerPixel(DXGI_FORMAT fmt)
{
	switch (static_cast<int>(fmt))
	{
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return 16;

	case DXGI_FORMAT_R32G32B32_TYPELESS:
	case DXGI_FORMAT_R32G32B32_FLOAT:
	case DXGI_FORMAT_R32G32B32_UINT:
	case DXGI_FORMAT_R32G32B32_SINT:
		return 12;

	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT:
	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT:
	case DXGI_FORMAT_R32G32_SINT:
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
	case DXGI_FORMAT_Y416:
	case DXGI_FORMAT_Y210:
	case DXGI_FORMAT_Y216:
		return 8;

	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT:
	case DXGI_FORMAT_R11G11B10_FLOAT:
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
	case DXGI_FORMAT_R16G16_TYPELESS:
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_UINT:
	case DXGI_FORMAT_R16G16_SNORM:
	case DXGI_FORMAT_R16G16_SINT:
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
	case DXGI_FORMAT_AYUV:
	case DXGI_FORMAT_Y410:
	case DXGI_FORMAT_YUY2:
		return 4;

	case DXGI_FORMAT_P010:
	case DXGI_FORMAT_P016:
		return 3;

	case DXGI_FORMAT_R8G8_TYPELESS:
	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R8G8_UINT:
	case DXGI_FORMAT_R8G8_SNORM:
	case DXGI_FORMAT_R8G8_SINT:
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SNORM:
	case DXGI_FORMAT_R16_SINT:
	case DXGI_FORMAT_B5G6R5_UNORM:
	case DXGI_FORMAT_B5G5R5A1_UNORM:
	case DXGI_FORMAT_A8P8:
	case DXGI_FORMAT_B4G4R4A4_UNORM:
		return 2;

	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SNORM:
	case DXGI_FORMAT_R8_SINT:
	case DXGI_FORMAT_A8_UNORM:
	case DXGI_FORMAT_AI44:
	case DXGI_FORMAT_IA44:
	case DXGI_FORMAT_P8:
	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
	case DXGI_FORMAT_R1_UNORM:
		return 1;

	default:
		return 0;
	}
}
