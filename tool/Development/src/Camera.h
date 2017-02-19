#pragma once

#include <DirectXMath.h>
#include <Windows.h>

#include "Transform.h"

class Camera
{
public:
	Camera();
	~Camera();

	DirectX::XMFLOAT4X4 ViewMatrix() const;
	DirectX::XMFLOAT4X4 ProjectionMatrix() const;
	
	void Update(float deltaTime);
	void Resize(unsigned int width, unsigned int height);
	Transform GetTransform();

	float fov, nearPlane, farPlane;
	
private:
	DirectX::XMFLOAT4X4 viewMat, projMat;
	TransformHandle transform;
	bool fpsEnabled;
	POINT fpsPos;
};