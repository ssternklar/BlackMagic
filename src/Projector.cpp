#include "Projector.h"

using namespace DirectX;

BlackMagic::Projector::Projector(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 direction, DirectX::XMFLOAT3 up, const std::shared_ptr<BlackMagic::Texture>& tex)
{
	D3D11_TEXTURE2D_DESC texDesc;
	ID3D11Texture2D* texIntfc;
	ID3D11Resource* resource;
	auto srv = tex->GetGraphicsTexture().GetAs<ID3D11ShaderResourceView*>();
	srv->GetResource(&resource);
	resource->QueryInterface<ID3D11Texture2D>(&texIntfc);
	texIntfc->GetDesc(&texDesc);
	resource->Release();
	texIntfc->Release();

	auto p = XMLoadFloat3(&pos);
	auto d = XMVector3Normalize(XMLoadFloat3(&direction));
	auto u = XMLoadFloat3(&up);
	//XMVectorSet(0.65,1,0.38,0)
	auto view = XMMatrixLookToLH(p, d, u);
	auto proj = XMMatrixOrthographicLH(2, 1, 0.1f, 1.0f);
	XMStoreFloat4x4(&_mat, XMMatrixTranspose(proj*view));
	_tex = tex;
}

XMFLOAT4X4 BlackMagic::Projector::Matrix() const
{
	return _mat;
}

BlackMagic::GraphicsTexture BlackMagic::Projector::Texture() const
{
	return _tex->GetGraphicsTexture();
}
