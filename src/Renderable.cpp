#include "Renderable.h"

using namespace BlackMagic;

Renderable::Renderable(const AssetPointer<Mesh>& mesh, const Material& mat)
	: _mesh(mesh),
	_material(mat)
{
	
}
