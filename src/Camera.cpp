#include "Camera.h"

#include <cmath>
#include <iostream>

#define KEYPRESSED(char) (GetAsyncKeyState(char) & 0x8000)

using namespace BlackMagic;

Camera::Camera() : pos(CreateVector3Zero()), offset(CreateVector3Zero())
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
	auto offsetV = quaternion * offset; 
	auto up = quaternion * CreateVector3(0, 1, 0);
	auto position = transform.GetPosition();
	auto view = CreateMatrixLookToLH(position + offsetV, transform.GetForward(), up);//XMMatrixLookToLH(position + offsetV, XMLoadFloat3(&transform.GetForward()), up);
	
	_viewMat = view;
	pos = position + offsetV;

	auto proj = _projMat;
	BoundingFrustum frustum{ proj };
	frustum.Transform(frustum, Inverse(view));
}

void Camera::UpdateProjectionMatrix(unsigned int width, unsigned int height)
{
	float aspect = static_cast<float>(width) / height;

	//using 2pi/5 instead of pi/4 for fov
	_projMat = CreateMatrixPerspectiveFovLH(0.4f * 3.14f, aspect, CAM_NEAR_Z, CAM_FAR_Z);
}
