#include "Transform.h"

#include "TransformData.h"

using namespace DirectX;

Transform::Transform()
	: _id(TransformData::AllocateTransform())
{
	TransformData::_positions[_id] = { 0, 0, 0 };
	TransformData::_rotations[_id] = { 0, 0, 0, 1 };
	TransformData::_scales[_id] = { 1, 1, 1 };
}

Transform::Transform(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 rotation, DirectX::XMFLOAT3 scale)
	: _id(TransformData::AllocateTransform())
{
	TransformData::_positions[_id] = pos;
	TransformData::_rotations[_id] = rotation;
	TransformData::_scales[_id] = scale;
}

Transform::~Transform()
{
	TransformData::DeallocateTransform(_id);
}

void Transform::Move(XMFLOAT3 dp)
{
	auto& ptr = TransformData::_positions[_id];
	ptr.x += dp.x;
	ptr.y += dp.y;
	ptr.z += dp.z;
}

void Transform::MoveTo(XMFLOAT3 pos)
{
	auto& ptr = TransformData::_positions[_id];
	ptr.x = pos.x;
	ptr.y = pos.y;
	ptr.z = pos.z;
}

void Transform::Rotate(XMFLOAT4 quaternion)
{
	auto current = XMLoadFloat4(&TransformData::_rotations[_id]);
	auto quat = XMLoadFloat4(&quaternion);
	XMStoreFloat4(&TransformData::_rotations[_id], XMQuaternionMultiply(current, quat));
}

void Transform::Rotate(XMFLOAT3 axis, float angle)
{
	auto axisVec = XMLoadFloat3(&axis);
	auto quat = XMQuaternionRotationAxis(axisVec, angle);
	auto current = XMLoadFloat4(&TransformData::_rotations[_id]);
	XMStoreFloat4(&TransformData::_rotations[_id], XMQuaternionMultiply(current, quat));
}

void Transform::SetRotation(XMFLOAT4 quaternion)
{
	TransformData::_rotations[_id] = quaternion;
}

void Transform::SetScale(XMFLOAT3 scale)
{
	TransformData::_scales[_id] = scale;
}

const DirectX::XMFLOAT4X4* Transform::Matrix()
{
	return TransformData::GetMatrix(_id);
}

XMFLOAT3 Transform::GetPosition()
{
	return TransformData::_positions[_id];
}

XMFLOAT3 Transform::GetScale()
{
	return TransformData::_scales[_id];
}

XMFLOAT4 Transform::GetRotation()
{
	return TransformData::_rotations[_id];
}

XMFLOAT3 Transform::GetForward()
{
	XMFLOAT3 ret;
	XMStoreFloat3(&ret, XMVector3Normalize(XMVector3Rotate(XMLoadFloat3(&TransformData::_positions[_id]), XMLoadFloat4(&TransformData::_rotations[_id]))));
	return ret;
}