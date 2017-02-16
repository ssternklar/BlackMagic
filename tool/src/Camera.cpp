#include "Camera.h"
#include "Input.h"

using namespace DirectX;

Camera::Camera()
{
	fov = 0.4f * XM_PI;
	nearPlane = 0.1f;
	farPlane = 100.0f;

	transform = TransformData::ptr->newTransform();
	TransformData::ptr->Move(transform, {0, 0, -3});

	Input::bindToControl("camForward", 'W');
	Input::bindToControl("camLeft", 'A');
	Input::bindToControl("camBack", 'S');
	Input::bindToControl("camRight", 'D');
	Input::bindToControl("camUp", VK_SPACE);
	Input::bindToControl("camDown", VK_CONTROL);
	Input::bindToControl("camSprint", VK_SHIFT);
	Input::bindToControl("camLook", VK_RBUTTON);
}

Camera::~Camera()
{
	TransformData::ptr->deleteTransform(transform);
}

DirectX::XMFLOAT4X4 Camera::ViewMatrix() const
{
	return viewMat;
}

DirectX::XMFLOAT4X4 Camera::ProjectionMatrix() const
{
	return projMat;
}

Transform Camera::GetTransform()
{
	return TransformData::ptr->GetTransform(transform);
}

void Camera::Update(float deltaTime)
{
	XMVECTOR offsetForward = XMVectorSet(0, 0, (float)(Input::isControlDown("camForward") - Input::isControlDown("camBack")), 1);
	XMVECTOR offsetUp = XMVectorSet(0, (float)(Input::isControlDown("camUp") - Input::isControlDown("camDown")), 0, 1);
	XMVECTOR offsetRight = XMVectorSet((float)(Input::isControlDown("camRight") - Input::isControlDown("camLeft")), 0, 0, 1);

	offsetForward = XMVector3Rotate(offsetForward, XMLoadFloat4(&TransformData::ptr->GetRotation(transform)));
	offsetRight = XMVector3Rotate(offsetRight, XMLoadFloat4(&TransformData::ptr->GetRotation(transform)));

	XMVECTOR offsetVec = offsetForward + offsetUp + offsetRight;

	if (Input::isControlDown("camDown"))
		printf("test\n");

	float speed = 15;
	if (Input::isControlDown("camSprint"))
		speed = 50;

	offsetVec = offsetVec * (speed * deltaTime);
	XMFLOAT3 offset;
	XMStoreFloat3(&offset, offsetVec);
	TransformData::ptr->Move(transform, offset);

	if (Input::isControlDown("camLook"))
	{
		TransformData::ptr->Rotate(transform, TransformData::ptr->GetRight(transform), Input::mouseOffset().y * 15 * deltaTime);
		TransformData::ptr->Rotate(transform, { 0,1,0 }, Input::mouseOffset().x * 15 * deltaTime);
	}

	XMVECTOR forward = XMLoadFloat3(&TransformData::ptr->GetForward(transform));
	XMVECTOR up = XMLoadFloat3(&TransformData::ptr->GetUp(transform));
	XMVECTOR position = XMLoadFloat3(&TransformData::ptr->GetPosition(transform));
	XMMATRIX view = XMMatrixLookToLH(position + forward, forward, up);
	XMStoreFloat4x4(&viewMat, XMMatrixTranspose(view));
}

void Camera::Resize(unsigned int width, unsigned int height)
{
	XMStoreFloat4x4(&projMat, XMMatrixTranspose(XMMatrixPerspectiveFovLH(fov, (float)width / height, nearPlane, farPlane)));
}