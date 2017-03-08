#include "Material.h"

Material::Material(
	const std::shared_ptr<SimpleVertexShader>& vs,
	const std::shared_ptr<SimplePixelShader>& ps,
	const std::shared_ptr<Texture>& albedo,
	const std::shared_ptr<Texture>& roughness,
	const std::shared_ptr<Texture>& metalness,
	const std::shared_ptr<Texture>& cavity,
	const std::shared_ptr<Texture>& normal,
	const ComPtr<ID3D11SamplerState>& sampler
)
	: _vertShader(vs),
	_pixelShader(ps),
	_albedo(albedo),
	_roughness(roughness),
	_metalness(metalness),
	_cavity(cavity),
	_normal(normal),
	_sampler(sampler)
{}


SimpleVertexShader* Material::VertexShader() const
{
	return _vertShader.get();
}

SimplePixelShader* Material::PixelShader() const
{
	return _pixelShader.get();
}

Texture* Material::Albedo() const
{
	return _albedo.get();
}

Texture* Material::NormalMap() const
{
	return _normal.get();
}

ID3D11SamplerState* Material::Sampler() const
{
	return _sampler.Get();
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

	_pixelShader->SetShaderResourceView("mainTex", _texture->GetGraphicsTexture().GetAs<ID3D11ShaderResourceView*>());
	_pixelShader->SetSamplerState("mainSampler", _mainSampler.Get());
	_pixelShader->SetShaderResourceView("normalMap", _normalMap->GetGraphicsTexture().GetAs<ID3D11ShaderResourceView*>());
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


