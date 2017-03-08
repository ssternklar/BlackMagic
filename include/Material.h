#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

#include "DirectionalLight.h"
#include "SimpleShader.h"
#include "Texture.h"

using BlackMagic::Texture;

enum AttributeStage : int
{
	VertexShader,	
	HullShader,
	DomainShader,
	GeometryShader,
	PixelShader,
	ComputeShader
};

class Material
{
public:
	Material(
		const std::shared_ptr<SimpleVertexShader>& vs, 
		const std::shared_ptr<SimplePixelShader>& ps,
		const std::shared_ptr<SimpleHullShader>& hs
	);

	SimpleVertexShader* VertexShader() const;
	SimplePixelShader* PixelShader() const;
	

	void Apply(
		DirectX::XMFLOAT4X4 view,
		DirectX::XMFLOAT4X4 proj);
	void Upload();

	bool operator ==(const Material&) const;
private:
	std::shared_ptr<SimpleVertexShader> _vertShader;
	std::shared_ptr<SimplePixelShader> _pixelShader;
	std::shared_ptr<Texture> _albedo, _roughness, _metalness, _cavity, _normal;
	ComPtr<ID3D11SamplerState> _sampler;
};
