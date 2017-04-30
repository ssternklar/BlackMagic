#pragma once

#include <DirectXMath.h>

#include "Patterns.h"

struct Transform
{
	DirectX::XMFLOAT4 rot;
	DirectX::XMFLOAT3 pos;
	float scale;
	DirectX::XMFLOAT4X4 matrix;
};

class TransformData : public ProxyHandler<Transform, TransformData>
{
public:
	Handle Get();
	void Revoke(Handle handle);
	void UpdateTransforms();

	DirectX::XMFLOAT3 GetForward(Handle handle);
	DirectX::XMFLOAT3 GetUp(Handle handle);
	DirectX::XMFLOAT3 GetRight(Handle handle);

	void Move(Handle handle, DirectX::XMFLOAT3 delta);
	void Rotate(Handle handle, DirectX::XMFLOAT4 quaternion);
	void Rotate(Handle handle, DirectX::XMFLOAT3 axis, float angle);
	void SetEuler(Handle handle, DirectX::XMFLOAT3 angles);
	DirectX::XMFLOAT3 GetEuler(Handle handle);
};