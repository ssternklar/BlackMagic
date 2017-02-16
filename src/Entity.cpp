#include "Entity.h"

using namespace BlackMagic;
using namespace DirectX;

Entity::Entity(XMFLOAT3 pos, XMFLOAT4 rot, std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> mat)
	: GameObject(pos, rot, {1, 1, 1}),
	Renderable(mesh, mat)
{}

const Renderable* Entity::AsRenderable() const
{
	return static_cast<const Renderable*>(this);
}

void Entity::Update()
{
	
}
