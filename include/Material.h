#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

#include "DirectionalLight.h"
#include "SimpleShader.h"
#include "Texture.h"

using BlackMagic::Texture;

enum ResourceStage : int
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
		const std::shared_ptr<SimpleHullShader>* hs = nullptr,
        const std::shared_ptr<SimpleDomainShader>* ds = nullptr,
        const std::shared_ptr<SimpleGeometryShader>* gs = nullptr,
        const std::shared_ptr<SimpleComputeShader>* cs = nullptr
	);

	SimpleVertexShader* VertexShader() const;
	SimplePixelShader* PixelShader() const;
    SimpleHullShader* HullShader() const;
    SimpleDomainShader* DomainShader() const;
    SimpleGeometryShader* GeometryShader() const;
    SimpleComputeShader* ComputeShader() const;

    //Turns on shader stages and uploads data
    void Use() const;

    template<typename T>
    void Set(ResourceStage s, T& data);

private:
    bool _active = false;
	std::shared_ptr<SimpleVertexShader> _vertShader;
	std::shared_ptr<SimplePixelShader> _pixelShader;
	std::shared_ptr<Texture> _albedo, _roughness, _metalness, _cavity, _normal;
	ComPtr<ID3D11SamplerState> _sampler;
};

#if defined(_WIN32) || defined(_WIN64)
template void Material::Set<int>(ResourceStage s, int& data);
template void Material::Set<float>(ResourceStage s, float& data);
template void Material::Set<DirectX::XMFLOAT2>(ResourceStage s, DirectX::XMFLOAT2& data);
template void Material::Set<DirectX::XMFLOAT3>(ResourceStage s, DirectX::XMFLOAT3& data);
template void Material::Set<DirectX::XMFLOAT4>(ResourceStage s, DirectX::XMFLOAT4& data);
template void Material::Set<DirectX::XMFLOAT4X4>(ResourceStage s, DirectX::XMFLOAT4X4& data);
template void Material::Set<ID3D11ShaderResourceView*>(ResourceStage s, ID3D11ShaderResourceView*& data);
template void Material::Set<ID3D11SamplerState*>(ResourceStage s, ID3D11SamplerState*& data);
#endif