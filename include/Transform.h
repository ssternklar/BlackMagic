#pragma once

#include <DirectXMath.h>

using TransformID = int32_t;

class Transform
{
public:
	Transform();
	Transform(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 orientation, DirectX::XMFLOAT3 scale);
	~Transform();

	void Move(DirectX::XMFLOAT3 dp);
	void MoveTo(DirectX::XMFLOAT3 pos);

	void Rotate(DirectX::XMFLOAT4 quaternion);
	void Rotate(DirectX::XMFLOAT3 axis, float angle);
	void SetRotation(DirectX::XMFLOAT4 quaternion);

	void SetScale(DirectX::XMFLOAT3 scale);
	const DirectX::XMFLOAT4X4* Matrix();
private:
	TransformID _id = -1;
};