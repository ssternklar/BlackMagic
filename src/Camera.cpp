#include "Camera.h"

#include <cmath>
#include <iostream>
#include <Windows.h>

#define KEYPRESSED(char) (GetAsyncKeyState(char) & 0x8000)

using namespace DirectX;

Camera::Camera(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 dir, float speed)
	: _pos(pos),
	_dir(dir),
	_speed(speed) {}

XMFLOAT3 Camera::Position() const
{
	return _pos;
}

XMFLOAT3 Camera::Direction() const
{
	return _dir;
}

XMFLOAT4X4 Camera::ViewMatrix() const
{
	return _viewMat;
}

XMFLOAT4X4 Camera::ProjectionMatrix() const
{
	return _projMat;
}

const DirectX::BoundingFrustum& Camera::Frustum() const
{
	return _frustum;
}


void Camera::Update(float dt)
{
	static bool keyWasPressed = false;

	//Only log position when a key was pressed last frame
	if (keyWasPressed)
		std::cout << "(" << _pos.x << ", " << _pos.y << ", " << _pos.z << ")" << std::endl;

	auto pos = XMLoadFloat3(&_pos);
	auto dir = XMLoadFloat3(&_dir);
	auto up = XMFLOAT3{0, 1, 0};
	auto defaultUp = XMLoadFloat3(&up);
	auto side = XMVector3Cross(defaultUp, dir);
	auto sideNorm = XMVector3Normalize(side);
	auto trueUp = XMVector3Cross(dir, sideNorm);

	keyWasPressed = false;
	if (KEYPRESSED(VK_LSHIFT))
	{
		_speed *= 2;
	}
	if (KEYPRESSED('W'))
	{
		pos += _speed * dt * dir;
		keyWasPressed = true;
	}
	if (KEYPRESSED('S'))
	{
		pos -= _speed * dt * dir;
		keyWasPressed = true;
	}
	if (KEYPRESSED('A'))
	{
		pos -= _speed * dt * sideNorm;
		keyWasPressed = true;
	}
	if (KEYPRESSED('D'))
	{
		pos += _speed * dt * sideNorm;
		keyWasPressed = true;
	}
	if (KEYPRESSED(VK_SPACE))
	{
		pos += _speed * dt * trueUp;
		keyWasPressed = true;
	}
	if (KEYPRESSED('X'))
	{
		pos -= _speed * dt * trueUp;
		keyWasPressed = true;
	}

	//If shift had been pressed, reset it 
	if (_speed > 10)
		_speed /= 2;

	//Store position back, it's not getting modified anymore
	XMStoreFloat3(&_pos, pos);

	//Calculate the rotation quaternion and reform the view matrix
	auto quat = XMQuaternionRotationRollPitchYaw(_rot.x, _rot.y, 0);
	auto fwd = XMFLOAT3{0, 0, 1};
	auto defaultForward = XMLoadFloat3(&fwd);
	auto rotatedFwd = XMVector3Rotate(defaultForward, quat);
	XMVector3Normalize(rotatedFwd);
	XMStoreFloat3(&_dir, rotatedFwd);

	side = XMVector3Cross(defaultUp, rotatedFwd);
	auto view = XMMatrixTranspose(XMMatrixLookToLH(pos, rotatedFwd, trueUp));
	XMStoreFloat4x4(&_viewMat, view);

	
	//Construct new viewing frustum for culling
	auto proj = XMLoadFloat4x4(&_projMat);
	BoundingFrustum newFrustum{ XMMatrixTranspose(proj) };
	newFrustum.Transform(_frustum, XMMatrixTranspose(XMMatrixInverse(nullptr, view)));
	
}

void Camera::Rotate(float x, float y)
{
	_rot.x += x;
	_rot.y += y;

	//Clamp maximum pitch so camera cannot flip around
	_rot.x = max(0.45f * -3.14f, min(0.45f * 3.14f, _rot.x));
	std::cout << "Camera: (" << _rot.x << ", " << _rot.y << ")" << std::endl;
}

void Camera::UpdateProjectionMatrix(int width, int height)
{
	float aspect = static_cast<float>(width) / height;

	//using 2pi/5 instead of pi/4 for fov
	auto newMat = XMMatrixTranspose(XMMatrixPerspectiveFovLH(CAM_FOV, aspect, CAM_NEAR_Z, CAM_FAR_Z));
	XMStoreFloat4x4(&_projMat, newMat);
}
