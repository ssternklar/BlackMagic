#pragma once

#include "Camera.h"
#include "GameObject.h"

namespace BlackMagic
{
	class FirstPersonCamera : public GameObject, public Camera
	{
	public:
		FirstPersonCamera(Vector3 pos, Quaternion dir);
		void Update(float delta);
	private:
		float _rotation[2];
	};
}