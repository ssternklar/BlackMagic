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

	transform = TransformData::Instance().Get();
	TransformData::Instance().Move(transform, {0, 0, -3});

	Input::BindToControl("camForward", 'W');
	Input::BindToControl("camLeft", 'A');
	Input::BindToControl("camBack", 'S');
	Input::BindToControl("camRight", 'D');
	Input::BindToControl("camUp", VK_SPACE);
	Input::BindToControl("camDown", VK_CONTROL);
	Input::BindToControl("camSprint", VK_SHIFT);
}

Camera::~Camera()
{
	TransformData::Instance().Revoke(transform);
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
	XMVECTOR offsetForward = XMVectorSet(0, 0, (float)(Input::IsControlDown("camForward") - Input::IsControlDown("camBack")), 1);
	XMVECTOR offsetUp = XMVectorSet(0, (float)(Input::IsControlDown("camUp") - Input::IsControlDown("camDown")), 0, 1);
	XMVECTOR offsetRight = XMVectorSet((float)(Input::IsControlDown("camRight") - Input::IsControlDown("camLeft")), 0, 0, 1);

	offsetForward = XMVector3Rotate(offsetForward, XMLoadFloat4(&transform->rot));
	offsetRight = XMVector3Rotate(offsetRight, XMLoadFloat4(&transform->rot));

	XMVECTOR offsetVec = offsetForward + offsetUp + offsetRight;

	float speed = 5;
	if (Input::IsControlDown("camSprint"))
		speed = 15;

	offsetVec = offsetVec * (speed * deltaTime);
	XMFLOAT3 offset;
	XMStoreFloat3(&offset, offsetVec);
	TransformData::Instance().Move(transform, offset);

	ImGuiIO& io = ImGui::GetIO();
	XMFLOAT2 delta = Input::getMouseDelta();
	if (io.MouseDown[1] && (delta.x != 0 || delta.y != 0))
	{
		TransformData::Instance().Rotate(transform, TransformData::Instance().GetRight(transform), delta.y * 2 * deltaTime);
		TransformData::Instance().Rotate(transform, { 0,1,0 }, delta.x * 2 * deltaTime);

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

	XMVECTOR forward = XMLoadFloat3(&TransformData::Instance().GetForward(transform));
	XMVECTOR up = XMLoadFloat3(&TransformData::Instance().GetUp(transform));
	XMVECTOR position = XMLoadFloat3(&transform->pos);
	XMMATRIX view = XMMatrixLookToLH(position, forward, up);
	XMStoreFloat4x4(&viewMat, XMMatrixTranspose(view));
}

void Camera::Resize(unsigned int width, unsigned int height)
{
	XMStoreFloat4x4(&projMat, XMMatrixTranspose(XMMatrixPerspectiveFovLH(fov, (float)width / height, nearPlane, farPlane)));
}