#include "FirstPersonCamera.h"

#include "WindowsPlatform.h"

using namespace BlackMagic;

#define KEYPRESSED(char) ((GetAsyncKeyState(char) & 0x8000)>>15)

FirstPersonCamera::FirstPersonCamera(Vector3 pos, Quaternion dir)
	: GameObject(pos, dir, {1,1,1}),
	Camera({0,0,0})
{
	Camera::Update(_transform);
}

void FirstPersonCamera::Update(float delta)
{
	auto input = WindowsPlatform::GetInstance()->GetInputData();

	if (input->GetButton(MouseButton::Left))
	{
		auto xRot = input->GetAxis(InputData::Axis::X)*0.5f*3.14f;
		auto yRot = input->GetAxis(InputData::Axis::Y)*0.5f*3.14f;

		//printf("(%f, %f)\n", xRot, yRot);

		_rotation[0] += xRot;
		_rotation[1] += yRot;
	
		_transform.SetRotation(CreateQuaternion(0, _rotation[1], _rotation[0]));
	}

	int boost = input->GetButton(Key::SHIFT);
	Vector3 dp = CreateVector3(
		delta * (input->GetButton(Key::D) - input->GetButton(Key::A)) * (1+boost) * 2,
		delta * (input->GetButton(Key::SPACE) - input->GetButton(Key::LCTRL)) * (1+boost) * 2,
		delta * (input->GetButton(Key::W) - input->GetButton(Key::S)) * (1+boost) * 2
	);

	_transform.Move(dp);

	Camera::Update(_transform);
}
