#include "FirstPersonCamera.h"

#include "WindowsPlatform.h"

using namespace BlackMagic;
using namespace DirectX;

#define KEYPRESSED(char) ((GetAsyncKeyState(char) & 0x8000)>>15)

FirstPersonCamera::FirstPersonCamera(XMFLOAT3 pos, XMFLOAT4 dir)
	: GameObject(pos, dir, {1,1,1}),
	Camera({0,0,0})
{
	Camera::Update(_transform);
}

void FirstPersonCamera::Update(float delta)
{
	auto input = WindowsPlatform::GetInstance()->GetInputData();

	if (input->GetButton(0))
	{
		auto xRot = input->GetAxis(InputData::Axis::X)*0.5f*XM_PI;
		auto yRot = input->GetAxis(InputData::Axis::Y)*0.5f*XM_PI;

		printf("(%f, %f)\n", xRot, yRot);

		_rotation.x += xRot;
		_rotation.y += yRot;
	
		xRot = max(0.45f*-XM_PI, min(0.45f*XM_PI, xRot));
		XMFLOAT4 q;
		XMStoreFloat4(&q, XMQuaternionRotationRollPitchYaw(_rotation.y, _rotation.x, 0));
		_transform.SetRotation(q);
	}

	int boost = KEYPRESSED(VK_LSHIFT);
	XMFLOAT3 dp = {
		delta * (KEYPRESSED('D') - KEYPRESSED('A')) * (1+boost) * 2,
		delta * (KEYPRESSED(VK_SPACE) - KEYPRESSED(VK_LCONTROL)) * (1+boost) * 2,
		delta * (KEYPRESSED('W') - KEYPRESSED('S')) * (1+boost) * 2
	};

	_transform.Move(dp);

	Camera::Update(_transform);
}
