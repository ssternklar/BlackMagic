#pragma once
#include <BMMath.h>
#include <DirectXCollision.h>
#include <Windows.h>

#include "Transform.h"

#define CAM_FOV (0.4f*3.14f)
#define CAM_NEAR_Z 0.1f
#define CAM_FAR_Z 100.0f

class Camera
{
public:
	Camera();
	Camera(BlackMagic::Vector3 offset);
	BlackMagic::Mat4 ViewMatrix() const;
	BlackMagic::Mat4 ProjectionMatrix() const;

	BlackMagic::Vector3 Position() const;
	BlackMagic::BoundingFrustum Frustum() const;
	void Update(Transform& transform);

	//Updates this camera's projection matrix to match a new window size
	void UpdateProjectionMatrix(unsigned int width, unsigned int height);

protected:
	BlackMagic::BoundingFrustum _frustum;
	BlackMagic::Vector3 pos, offset;
	BlackMagic::Mat4 _viewMat, _projMat;
	float _speed;
};
