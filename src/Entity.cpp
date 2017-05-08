#include "Entity.h"

using namespace BlackMagic;

Entity::Entity(const Vector3& pos, const Quaternion& rot, const AssetPointer<Mesh>& mesh, const Material& mat)
	: GameObject(pos, rot, {1, 1, 1}),
	Renderable(mesh, mat)
{}

const Renderable* Entity::AsRenderable() const
{
	return static_cast<const Renderable*>(this);
}

void Entity::Update(float deltaTime)
{
	
}
