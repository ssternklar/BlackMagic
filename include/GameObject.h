#pragma once

#include <DirectXMath.h>
#include "Transform.h"

namespace BlackMagic
{
	class GameObject
	{
	public:
		GameObject() = default;
		GameObject(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 rot, DirectX::XMFLOAT3 scale);

		Transform& GetTransform();
	protected:
		Transform _transform;
	};
}