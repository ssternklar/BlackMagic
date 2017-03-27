#include "GameObject.h"

using namespace BlackMagic;
using namespace DirectX;

GameObject::GameObject(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 rot, DirectX::XMFLOAT3 scale)
	: _transform(pos, rot, scale)
{}

Transform& GameObject::GetTransform()
{
	return _transform;
}

