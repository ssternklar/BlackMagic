#include "Camera.h"

#include <cmath>
#include <iostream>

#define KEYPRESSED(char) (GetAsyncKeyState(char) & 0x8000)

using namespace BlackMagic;

Camera::Camera() : offset(CreateVector3Zero()), pos(CreateVector3Zero())
{
}

Camera::Camera(Vector3 offset) : offset(offset), pos({0,0,0})
{
}

Mat4 Camera::ViewMatrix() const
{
	return _viewMat;
}

Mat4 Camera::ProjectionMatrix() const
{
	return _projMat;
}

Vector3 Camera::Position() const
{
	return pos;
}

BoundingFrustum Camera::Frustum() const
{
	return _frustum;
}

void Camera::Update(Transform& transform)
{
	auto quaternion = transform.GetRotation();
	auto offsetV = Rotate(offset, quaternion); 
	auto up = Rotate(CreateVector3(0, 1, 0), quaternion);
	auto position = transform.GetPosition();
	auto view = CreateMatrixLookToLH(position + offsetV, transform.GetForward(), up);//XMMatrixLookToLH(position + offsetV, XMLoadFloat3(&transform.GetForward()), up);
	
	_viewMat = Transpose(view);
	pos = position + offsetV;

	auto proj = Transpose(_projMat);
	BoundingFrustum frustum{ proj };
	frustum.Transform(frustum, Inverse(view));
}

void Camera::UpdateProjectionMatrix(UINT width, UINT height)
{
	float aspect = static_cast<float>(width) / height;

	//using 2pi/5 instead of pi/4 for fov
	_projMat = Transpose(CreateMatrixPerspectiveFovLH(0.4f * 3.14f, aspect, CAM_NEAR_Z, CAM_FAR_Z));
}
