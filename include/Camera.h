#pragma once
#include <DirectXMath.h>
#include "Transform.h"

class Camera
{
public:
	Camera();
	Camera(DirectX::XMFLOAT3 offset);
	DirectX::XMFLOAT4X4 ViewMatrix() const;
	DirectX::XMFLOAT4X4 ProjectionMatrix() const;

	DirectX::XMFLOAT3 Position() const;
	void Update(Transform* transform);

	//Updates this camera's projection matrix to match a new window size
	void UpdateProjectionMatrix(int width, int height);

private:
	DirectX::XMFLOAT3 pos, offset;
	DirectX::XMFLOAT4X4 _viewMat, _projMat;
	float _speed;
};
