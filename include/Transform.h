#pragma once

#include <BMMath.h>

using TransformID = size_t;

class Transform
{
public:
	Transform();
	Transform(BlackMagic::Vector3 pos, BlackMagic::Quaternion orientation, BlackMagic::Vector3 scale);
	~Transform();

	void Move(BlackMagic::Vector3 dp);
	void MoveTo(BlackMagic::Vector3 pos);

	void Rotate(BlackMagic::Quaternion q);
	void Rotate(BlackMagic::Vector3 axis, float angle);
	void SetRotation(BlackMagic::Quaternion quaternion);

	void SetScale(BlackMagic::Vector3 scale);
	BlackMagic::Mat4* Matrix();

	BlackMagic::Vector3 GetPosition();
	BlackMagic::Vector3 GetScale();
	BlackMagic::Quaternion GetRotation();
	BlackMagic::Vector3 GetForward();
private:
	TransformID _id = -1;
};