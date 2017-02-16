#pragma once

#include "Camera.h"
#include "GameObject.h"

#include "Windows.h"

namespace BlackMagic
{
	class FirstPersonCamera : public GameObject, public Camera
	{
	public:
		FirstPersonCamera(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 dir);
		void Update(float delta);
	private:
		DirectX::XMFLOAT2 _rotation;
	};
}