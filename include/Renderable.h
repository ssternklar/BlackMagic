#pragma once

#include <memory>
#include "Mesh.h"
#include "Material.h"
#include "Transform.h"
#include "ContentClasses.h"
namespace BlackMagic
{
	class Renderable
	{
	public:
		Renderable() = default;
		Renderable(const AssetPointer<BlackMagic::Mesh>& mesh, const Material& mat);

		AssetPointer<BlackMagic::Mesh> _mesh;
		Material _material;
	};	
}
