#pragma once

#include "Camera.h"
#include "GameObject.h"

class FirstPersonCamera : public GameObject, public Camera
{
public:
	FirstPersonCamera(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 dir);
	void Update();
};