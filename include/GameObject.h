#pragma once

#include <DirectXMath.h>
#include "Transform.h"

class GameObject
{
public:
	GameObject(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 rot, DirectX::XMFLOAT3 scale);

	Transform& GetTransform();
protected:
	Transform _transform;
};