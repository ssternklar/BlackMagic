#pragma once
#include "GameObject.h"
#include "Renderable.h"

namespace BlackMagic
{
	class Entity : public GameObject, public Renderable
	{
	public:
		Entity() = default;
		Entity(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 rot, const std::shared_ptr<Mesh> mesh, const std::shared_ptr<Material> mat);

		const Renderable* AsRenderable() const;
		void Update();
	private:
	};	
}