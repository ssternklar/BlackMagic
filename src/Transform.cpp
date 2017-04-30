#include "Transform.h"

using namespace BlackMagic;

Transform::Transform()
{
	position = { 0, 0, 0 };
	rotation = { 0, 0, 0, 1 };
	scale = { 1, 1, 1 };
}

Transform::Transform(BlackMagic::Vector3 pos, BlackMagic::Quaternion rotation, BlackMagic::Vector3 scale)
{
	position = pos;
	this->rotation = rotation;
	this->scale = scale;
}

Transform::Transform(const Transform & other)
{
	position = other.position;
	rotation = other.rotation;
	scale = other.scale;
}

void Transform::Move(Vector3 dp)
{
	Vector3 rotated = rotation * dp;

	position = position + rotated;
}

void Transform::MoveTo(Vector3 pos)
{
	position = position + pos;
}

void Transform::Rotate(Quaternion q)
{
	auto& current = rotation;
	current = q * current;
}

void Transform::Rotate(Vector3 axis, float angle)
{
	auto quat = CreateQuaternion(axis, angle);
	auto& current = rotation;
	current = quat * current;
}

void Transform::SetRotation(Quaternion quaternion)
{
	this->rotation = rotation;
}

void Transform::SetScale(Vector3 scale)
{
	this->scale = scale;
}

BlackMagic::Mat4 Transform::Matrix()
{
	auto rS = CreateVector3Zero();
	auto mat = CreateAffineTransformation(
		scale,
		rS,
		rotation,
		position
	);

	return Transpose(mat);
}

Vector3 Transform::GetPosition()
{
	return position;
}

Vector3 Transform::GetScale()
{
	return scale;
}

Quaternion Transform::GetRotation()
{
	return rotation;
}

Vector3 Transform::GetForward()
{
	return rotation * CreateVector3(0, 0, 1);
}