#include "Transform.h"

#include "TransformData.h"

using namespace BlackMagic;

Transform::Transform()
	: _id(TransformData::GetSingleton()->AllocateTransform())
{
	TransformData::GetSingleton()->_positions[_id] = { 0, 0, 0 };
	TransformData::GetSingleton()->_rotations[_id] = { 0, 0, 0, 1 };
	TransformData::GetSingleton()->_scales[_id] = { 1, 1, 1 };
}

Transform::Transform(BlackMagic::Vector3 pos, BlackMagic::Quaternion rotation, BlackMagic::Vector3 scale)
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

void Transform::Move(Vector3 dp)
{
	auto& ptr = TransformData::GetSingleton()->_positions[_id];
	Vector3 rotated = BlackMagic::Rotate(dp, TransformData::GetSingleton()->_rotations[_id]);

	ptr = ptr + rotated;
}

void Transform::MoveTo(Vector3 pos)
{
	auto& ptr = TransformData::GetSingleton()->_positions[_id];
	ptr = ptr + pos;
}

void Transform::Rotate(Quaternion q)
{
	auto& current = TransformData::GetSingleton()->_rotations[_id];
	current = q * current;
}

void Transform::Rotate(Vector3 axis, float angle)
{
	auto quat = CreateQuaternion(axis, angle);
	auto& current = TransformData::GetSingleton()->_rotations[_id];
	current = quat * current;
}

void Transform::SetRotation(Quaternion quaternion)
{
	TransformData::GetSingleton()->_rotations[_id] = quaternion;
}

void Transform::SetScale(Vector3 scale)
{
	TransformData::GetSingleton()->_scales[_id] = scale;
}

BlackMagic::Mat4* Transform::Matrix()
{
	return TransformData::GetSingleton()->GetMatrix(_id);
}

Vector3 Transform::GetPosition()
{
	return TransformData::GetSingleton()->_positions[_id];
}

Vector3 Transform::GetScale()
{
	return TransformData::GetSingleton()->_scales[_id];
}

Quaternion Transform::GetRotation()
{
	return TransformData::GetSingleton()->_rotations[_id];
}

Vector3 Transform::GetForward()
{
	return BlackMagic::Rotate(CreateVector3(0, 0, 1), TransformData::GetSingleton()->_rotations[_id]);
}