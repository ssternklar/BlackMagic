#include "Camera.h"

#include <cmath>
#include <iostream>
#include <Windows.h>

#define KEYPRESSED(char) (GetAsyncKeyState(char) & 0x8000)

using namespace DirectX;

Camera::Camera() : offset({ 0,0,0 }), pos({0,0,0})
{
}

Camera::Camera(XMFLOAT3 offset) : offset(offset), pos({0,0,0})
{
}

XMFLOAT4X4 Camera::ViewMatrix() const
{
	return _viewMat;
}

XMFLOAT4X4 Camera::ProjectionMatrix() const
{
	return _projMat;
}

DirectX::XMFLOAT3 Camera::Position() const
{
	return pos;
}

BoundingFrustum Camera::Frustum() const
{
	return _frustum;
}

void Camera::Update(Transform* transform)
{
	auto quaternion = XMLoadFloat4(&transform->GetRotation());
	auto offsetV = XMVector3Rotate(XMLoadFloat3(&offset), quaternion);
	auto up = XMVector3Rotate(XMVectorSet(0, 1, 0, 0), quaternion);
	auto position = XMLoadFloat3(&transform->GetPosition());
	auto view = XMMatrixLookToLH(position + offsetV, XMLoadFloat3(&transform->GetForward()), up);
	XMStoreFloat4x4(&_viewMat, XMMatrixTranspose(view));
	XMStoreFloat3(&pos, position + offsetV);

	auto proj = XMMatrixTranspose(XMLoadFloat4x4(&_projMat));
	BoundingFrustum frustum{ proj };
	frustum.Transform(frustum, XMMatrixInverse(nullptr, view));
}

void Camera::UpdateProjectionMatrix(int width, int height)
{
	float aspect = static_cast<float>(width) / height;

	//using 2pi/5 instead of pi/4 for fov
	XMStoreFloat4x4(&_projMat,
	                XMMatrixTranspose(XMMatrixPerspectiveFovLH(0.4f * 3.14f, aspect, 0.1f, 100)));
}
