#include "Renderer.h"

Renderer::Renderer(ID3D11Device* device, ID3D11DeviceContext* ctx)
	: _device(device),
	_context(ctx)
{}

void Renderer::Render(const Camera& cam, const std::vector<Renderable*>& objects, const std::vector<DirectionalLight> lights)
{
	static UINT stride = sizeof(Vertex);
	static UINT offset = 0;

	Material* currentMaterial = nullptr;

	//TODO: Sort renderables by material and texture to minimize state switches
	for(auto* object : objects)
	{
		if (object->_material.get() != currentMaterial)
		{
			currentMaterial = object->_material.get();
			object->_material->UpdateLights(lights);
		}
		
		object->_material->PixelShader()->SetData("UseNormalMap", &UseNormalMap, sizeof(unsigned int));
		object->PrepareMaterial(cam.ViewMatrix(), cam.ProjectionMatrix());
		//Upload buffers and draw
		
		auto vBuf = object->_mesh->VertexBuffer();
		_context->IASetVertexBuffers(0, 1, &vBuf, &stride, &offset);
		_context->IASetIndexBuffer(object->_mesh->IndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
		_context->DrawIndexed(static_cast<UINT>(object->_mesh->IndexCount()), 0, 0);
	}
}

