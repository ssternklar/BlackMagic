#pragma once

#include <d3d11.h>
#include <memory>
#include "Mesh.h"
#include "Material.h"
#include "Transform.h"

class Renderable
{
public:
	Renderable() = default;
	Renderable(const std::shared_ptr<BlackMagic::Mesh>& mesh, const std::shared_ptr<Material>& mat);

	void PrepareMaterial(Transform& transform, DirectX::XMFLOAT4X4 view, DirectX::XMFLOAT4X4 proj);

	std::shared_ptr<BlackMagic::Mesh> _mesh;
	std::shared_ptr<Material> _material;
};
