#include "Transform.h"

#include "TransformData.h"

using namespace BlackMagic;
using namespace DirectX;

Transform::Transform()
	: _id(TransformData::GetSingleton()->AllocateTransform())
{
	TransformData::GetSingleton()->_positions[_id] = { 0, 0, 0 };
	TransformData::GetSingleton()->_rotations[_id] = { 0, 0, 0, 1 };
	TransformData::GetSingleton()->_scales[_id] = { 1, 1, 1 };
}

Transform::Transform(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 rotation, DirectX::XMFLOAT3 scale)
	: _id(TransformData::GetSingleton()->AllocateTransform())
{
	TransformData::GetSingleton()->_positions[_id] = pos;
	TransformData::GetSingleton()->_rotations[_id] = rotation;
	TransformData::GetSingleton()->_scales[_id] = scale;
}

Transform::~Transform()
{
	TransformData::GetSingleton()->DeallocateTransform(_id);
}

void Transform::Move(XMFLOAT3 dp)
{
	auto& ptr = TransformData::GetSingleton()->_positions[_id];

	auto v = XMLoadFloat3(&dp);
	auto q = XMLoadFloat4(&TransformData::GetSingleton()->_rotations[_id]);
	XMFLOAT3 rotated;
	XMStoreFloat3(&rotated, XMVector3Rotate(v, q));

	ptr.x += rotated.x;
	ptr.y += rotated.y;
	ptr.z += rotated.z;
}

void Transform::MoveTo(XMFLOAT3 pos)
{
	auto& ptr = TransformData::GetSingleton()->_positions[_id];
	ptr.x = pos.x;
	ptr.y = pos.y;
	ptr.z = pos.z;
}

void Transform::Rotate(XMFLOAT4 quaternion)
{
	auto current = XMLoadFloat4(&TransformData::GetSingleton()->_rotations[_id]);
	auto quat = XMLoadFloat4(&quaternion);
	XMStoreFloat4(&TransformData::GetSingleton()->_rotations[_id], XMQuaternionMultiply(quat, current));
}

void Transform::Rotate(XMFLOAT3 axis, float angle)
{
	auto axisVec = XMLoadFloat3(&axis);
	auto quat = XMQuaternionRotationAxis(axisVec, angle);
	auto current = XMLoadFloat4(&TransformData::GetSingleton()->_rotations[_id]);
	XMStoreFloat4(&TransformData::GetSingleton()->_rotations[_id], XMQuaternionMultiply(current, quat));
}

void Transform::SetRotation(XMFLOAT4 quaternion)
{
	TransformData::GetSingleton()->_rotations[_id] = quaternion;
}

void Transform::SetScale(XMFLOAT3 scale)
{
	TransformData::GetSingleton()->_scales[_id] = scale;
}

DirectX::XMFLOAT4X4* Transform::Matrix()
{
	return TransformData::GetSingleton()->GetMatrix(_id);
}

XMFLOAT3 Transform::GetPosition()
{
	return TransformData::GetSingleton()->_positions[_id];
}

XMFLOAT3 Transform::GetScale()
{
	return TransformData::GetSingleton()->_scales[_id];
}

XMFLOAT4 Transform::GetRotation()
{
	return TransformData::GetSingleton()->_rotations[_id];
}

XMFLOAT3 Transform::GetForward()
{
	XMFLOAT3 ret;
	XMStoreFloat3(&ret, XMVector3Rotate(XMVectorSet(0,0,1,0), XMLoadFloat4(&TransformData::GetSingleton()->_rotations[_id])));
	return ret;
}