#pragma once

#include <d3d11.h>
#include <memory>
#include "Mesh.h"
#include "Material.h"
#include "Transform.h"

namespace BlackMagic
{
	class Renderable
	{
	public:
		Renderable() = default;
		Renderable(const std::shared_ptr<BlackMagic::Mesh>& mesh, const Material& mat);

		std::shared_ptr<BlackMagic::Mesh> _mesh;
		Material _material;
	};	
}
