#include "GameObject.h"

using namespace DirectX;

GameObject::GameObject(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 rot, DirectX::XMFLOAT3 scale)
{
	_transform.MoveTo(pos);
	_transform.SetRotation(rot);
	_transform.SetScale(scale);
}

Transform& GameObject::GetTransform()
{
	return _transform;
}

