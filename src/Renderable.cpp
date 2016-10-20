#include "Renderable.h"

using namespace DirectX;

Renderable::Renderable(const std::shared_ptr<Mesh> mesh, const std::shared_ptr<Material> mat,
	XMFLOAT3 pos, XMFLOAT4 rot, XMFLOAT3 scale)
	: transform(pos, rot, scale),
	_mesh(mesh),
	_material(mat)
{
	
}

void Renderable::PrepareMaterial(XMFLOAT4X4 view, XMFLOAT4X4 proj)
{
	auto vs = _material->VertexShader();
	auto ps = _material->PixelShader();

	vs->SetMatrix4x4("world", *transform.Matrix());
	vs->SetMatrix4x4("view", view);
	vs->SetMatrix4x4("projection", proj);
	
	ps->SetShaderResourceView("mainTex", *_material->MainTexture());
	ps->SetSamplerState("mainSampler", _material->MainSampler());
	ps->SetShaderResourceView("normalMap", *_material->NormalMap());
	
	vs->SetShader();
	ps->SetShader();

	vs->CopyAllBufferData();
	ps->CopyAllBufferData();
}
