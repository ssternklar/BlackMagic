#pragma once
#include "GameObject.h"
#include "Renderable.h"
#include "ContentClasses.h"
namespace BlackMagic
{
	class Entity : public GameObject, public Renderable
	{
	public:
		Entity() = default;
		Entity(const Vector3& pos, const Quaternion& rot, const AssetPointer<Mesh>& mesh, const Material& mat);
		virtual ~Entity() = default;

		const Renderable* AsRenderable() const;
		virtual void Update(float deltaTime);
	private:
	};	
}