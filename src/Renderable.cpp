#include "Renderable.h"

using namespace DirectX;

Renderable::Renderable(const std::shared_ptr<Mesh> mesh, const std::shared_ptr<Material> mat)
	: _mesh(mesh),
	_material(mat)
{
	
}

Renderable::Renderable()
{

}

void Renderable::PrepareMaterial(Transform& transform, XMFLOAT4X4 view, XMFLOAT4X4 proj)
{
	auto vs = _material->VertexShader();
	auto ps = _material->PixelShader();

	vs->SetMatrix4x4("world", *transform.Matrix());
	vs->SetMatrix4x4("view", view);
	vs->SetMatrix4x4("projection", proj);
	
	ps->SetShaderResourceView("mainTex", _material->MainTexture()->GetGraphicsTexture().GetAs<ID3D11ShaderResourceView*>());
	ps->SetSamplerState("mainSampler", _material->MainSampler());
	ps->SetShaderResourceView("normalMap", _material->NormalMap()->GetGraphicsTexture().GetAs<ID3D11ShaderResourceView*>());
	
	vs->SetShader();
	ps->SetShader();

	vs->CopyAllBufferData();
	ps->CopyAllBufferData();
}
