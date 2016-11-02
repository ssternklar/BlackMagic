#include "Material.h"

Material::Material(
	const std::shared_ptr<SimpleVertexShader>& vs,
	const std::shared_ptr<SimplePixelShader>& ps,
	const std::shared_ptr<Texture>& tex,
	const std::shared_ptr<ID3D11SamplerState>& sampler,
	const std::shared_ptr<Texture>& normalMap
)
	: _vertShader(vs),
	_pixelShader(ps), 
	_texture(tex),
	_mainSampler(sampler),
	_normalMap(normalMap)
{}


SimpleVertexShader* Material::VertexShader() const
{
	return _vertShader.get();
}

SimplePixelShader* Material::PixelShader() const
{
	return _pixelShader.get();
}

Texture* Material::MainTexture() const
{
	return _texture.get();
}

Texture* Material::NormalMap() const
{
	return _normalMap.get();
}

ID3D11SamplerState* Material::MainSampler() const
{
	return _mainSampler.get();
}

void Material::Apply(
	DirectX::XMFLOAT4X4 view,
	DirectX::XMFLOAT4X4 proj)
{
	//Find how many bytes of padding DirectX will add to the constant buffer
	//DX will only pad in-between consecutive structs and not on the end
	//size_t padding = (16 - (sizeof(DirectionalLight) % 16))*(lights.size() - 1);

	//Set per-frame material attributes
	_vertShader->SetMatrix4x4("view", view);
	_vertShader->SetMatrix4x4("projection", proj);

	_pixelShader->SetShaderResourceView("mainTex", *_texture);
	_pixelShader->SetSamplerState("mainSampler", _mainSampler.get());
	_pixelShader->SetShaderResourceView("normalMap", *_normalMap);
	//_pixelShader->SetData("directionalLights", &lights[0], sizeof(DirectionalLight)*lights.size() + padding);

	_vertShader->SetShader();
	_pixelShader->SetShader();
}

void Material::Upload()
{
	_vertShader->CopyAllBufferData();
	_pixelShader->CopyAllBufferData();
}

bool Material::operator==(const Material& mat) const
{
	return (_vertShader == mat._vertShader 
		&& _pixelShader == mat._pixelShader
		&& _texture == mat._texture
		&& _normalMap == mat._normalMap
		&& _mainSampler == mat._mainSampler);
}


