#pragma once

#include <d3d11.h>
#include <memory>
#include "Mesh.h"
#include "Material.h"
#include "Transform.h"

class Renderable
{
	friend class GraphicsDevice;
public:
	Transform transform;
	Renderable(const std::shared_ptr<Mesh> mesh, const std::shared_ptr<Material> mat,
		DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 rot, DirectX::XMFLOAT3 scale);

	void PrepareMaterial(DirectX::XMFLOAT4X4 view, DirectX::XMFLOAT4X4 proj);

protected:
	std::shared_ptr<Mesh> _mesh;
	std::shared_ptr<Material> _material;
};
