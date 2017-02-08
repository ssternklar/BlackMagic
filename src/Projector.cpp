/*
#include "Projector.h"

using namespace DirectX;

BlackMagic::Projector::Projector(SplineControlPoint cp, const std::shared_ptr<BlackMagic::Texture>& tex)
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

	auto p = XMLoadFloat3(&cp.position);
	auto d = XMVector3Normalize(XMLoadFloat3(&cp.normal));
	auto u = XMLoadFloat3(&cp.tangent);
	auto view = XMMatrixLookAtLH(p + d, p, u);
	auto proj = XMMatrixOrthographicLH(4, 2, 0.1f, 1.1f);
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

*/