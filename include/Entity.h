#pragma once
#include "GameObject.h"
#include "Renderable.h"

namespace BlackMagic
{
	class Entity : public GameObject, public Renderable
	{
	public:
		Entity() = default;
		Entity(Vector3 pos, Quaternion rot, const std::shared_ptr<Mesh> mesh, const Material& mat);

		const Renderable* AsRenderable() const;
		void Update();
	private:
	};	
}