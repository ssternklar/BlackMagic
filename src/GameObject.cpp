#include "GameObject.h"

using namespace BlackMagic;

GameObject::GameObject(Vector3 pos, Quaternion rot, Vector3 scale)
	: _transform(pos, rot, scale)
{}

Transform& GameObject::GetTransform()
{
	return _transform;
}

