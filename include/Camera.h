#pragma once
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include "Transform.h"

#define CAM_FOV (0.4f*3.14f)
#define CAM_NEAR_Z 0.1f
#define CAM_FAR_Z 100.0f

class Camera
{
public:
	Camera();
	Camera(DirectX::XMFLOAT3 offset);
	DirectX::XMFLOAT4X4 ViewMatrix() const;
	DirectX::XMFLOAT4X4 ProjectionMatrix() const;

	DirectX::XMFLOAT3 Position() const;
	DirectX::BoundingFrustum Frustum() const;
	void Update(Transform* transform);

	//Updates this camera's projection matrix to match a new window size
	void UpdateProjectionMatrix(int width, int height);

private:
	DirectX::BoundingFrustum _frustum;
	DirectX::XMFLOAT3 pos, offset;
	DirectX::XMFLOAT4X4 _viewMat, _projMat;
	float _speed;
};
