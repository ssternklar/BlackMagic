#include "Renderable.h"

using namespace BlackMagic;

Renderable::Renderable(const std::shared_ptr<Mesh>& mesh, const Material& mat)
	: _mesh(mesh),
	_material(mat)
{
	
}
