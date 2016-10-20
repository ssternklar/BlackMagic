#include "Entity.h"

Entity::Entity(const std::shared_ptr<Mesh>& mesh,
			   const std::shared_ptr<Material>& material,
			   XMFLOAT3 pos,
			   XMFLOAT4 rot,
			   XMFLOAT3 scale)
	: Renderable(mesh, material, pos, rot, scale)
{
	
}

void Entity::Update()
{
	
}
