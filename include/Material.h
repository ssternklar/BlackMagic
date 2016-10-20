#pragma once

#include <d3d11.h>
#include <memory>

#include "DirectionalLight.h"
#include "SimpleShader.h"
#include "Texture.h"

class Material
{
public:
	Material(
		const std::shared_ptr<SimpleVertexShader>& vs, 
		const std::shared_ptr<SimplePixelShader>& ps,
		const std::shared_ptr<Texture>& tex,
		const std::shared_ptr<ID3D11SamplerState>& sampler,
		const std::shared_ptr<Texture>& normalMap
	);

	SimpleVertexShader* VertexShader() const;
	SimplePixelShader* PixelShader() const;
	Texture* MainTexture() const;
	Texture* NormalMap() const;
	ID3D11SamplerState* MainSampler() const;

	void UpdateLights(const std::vector<DirectionalLight> lights);

	bool operator ==(const Material&) const;
private:
	std::shared_ptr<SimpleVertexShader> _vertShader;
	std::shared_ptr<SimplePixelShader> _pixelShader;
	std::shared_ptr<Texture> _texture, _normalMap;
	std::shared_ptr<ID3D11SamplerState> _mainSampler;
};
