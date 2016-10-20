#pragma once

#include <memory>
#include <vector>
#include "Mesh.h"
#include "Material.h"
#include "DirectionalLight.h"
#include "Renderable.h"

using DirectX::XMFLOAT3;
using DirectX::XMFLOAT4;
using DirectX::XMFLOAT4X4;

class Entity : public Renderable
{
public:
	//Constructs an entity given a mesh and starting transformation data
	Entity(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Material>& material, XMFLOAT3 pos, XMFLOAT4 rot, XMFLOAT3 scale);
	void Update();
};
