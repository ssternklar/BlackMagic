#pragma once

#include "BMMath.h"
#include "Transform.h"

namespace BlackMagic
{
	class GameObject
	{
	public:
		GameObject() = default;
		GameObject(Vector3 pos, Quaternion rot, Vector3 scale);
		virtual ~GameObject() = default;

		Transform& GetTransform();
	protected:
		Transform _transform;
	};
}