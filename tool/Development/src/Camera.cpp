#include "Camera.h"
#include "Input.h"
#include "dear imgui\imgui.h"

using namespace DirectX;

Camera::Camera()
{
	fov = 0.4f * XM_PI;
	nearPlane = 0.1f;
	farPlane = 100.0f;

	fpsEnabled = false;
	fpsPos = { 0, 0 };

	transform = TransformData::ptr->newTransform();
	TransformData::ptr->Move(transform, {0, 0, -3});

	Input::bindToControl("camForward", 'W');
	Input::bindToControl("camLeft", 'A');
	Input::bindToControl("camBack", 'S');
	Input::bindToControl("camRight", 'D');
	Input::bindToControl("camUp", VK_SPACE);
	Input::bindToControl("camDown", VK_CONTROL);
	Input::bindToControl("camSprint", VK_SHIFT);
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
	return *transform;
}

void Camera::Update(float deltaTime)
{
	XMVECTOR offsetForward = XMVectorSet(0, 0, (float)(Input::isControlDown("camForward") - Input::isControlDown("camBack")), 1);
	XMVECTOR offsetUp = XMVectorSet(0, (float)(Input::isControlDown("camUp") - Input::isControlDown("camDown")), 0, 1);
	XMVECTOR offsetRight = XMVectorSet((float)(Input::isControlDown("camRight") - Input::isControlDown("camLeft")), 0, 0, 1);

	offsetForward = XMVector3Rotate(offsetForward, XMLoadFloat4(&transform->rot));
	offsetRight = XMVector3Rotate(offsetRight, XMLoadFloat4(&transform->rot));

	XMVECTOR offsetVec = offsetForward + offsetUp + offsetRight;

	float speed = 5;
	if (Input::isControlDown("camSprint"))
		speed = 15;

	offsetVec = offsetVec * (speed * deltaTime);
	XMFLOAT3 offset;
	XMStoreFloat3(&offset, offsetVec);
	TransformData::ptr->Move(transform, offset);

	ImGuiIO& io = ImGui::GetIO();
	XMFLOAT2 delta = Input::getMouseDelta();
	if (io.MouseDown[1] && (delta.x != 0 || delta.y != 0))
	{
		TransformData::ptr->Rotate(transform, TransformData::ptr->GetRight(transform), delta.y * 2 * deltaTime);
		TransformData::ptr->Rotate(transform, { 0,1,0 }, delta.x * 2 * deltaTime);

		if (!fpsEnabled)
		{
			ShowCursor(false);
			fpsEnabled = true;
			GetCursorPos(&fpsPos);
		}

		SetCursorPos(fpsPos.x, fpsPos.y);
	}

	if (io.MouseReleased[1] && fpsEnabled)
	{
		ShowCursor(true);
		fpsEnabled = false;
	}

	XMVECTOR forward = XMLoadFloat3(&TransformData::ptr->GetForward(transform));
	XMVECTOR up = XMLoadFloat3(&TransformData::ptr->GetUp(transform));
	XMVECTOR position = XMLoadFloat3(&transform->pos);
	XMMATRIX view = XMMatrixLookToLH(position + forward, forward, up);
	XMStoreFloat4x4(&viewMat, XMMatrixTranspose(view));
}

void Camera::Resize(unsigned int width, unsigned int height)
{
	XMStoreFloat4x4(&projMat, XMMatrixTranspose(XMMatrixPerspectiveFovLH(fov, (float)width / height, nearPlane, farPlane)));
}