#pragma once

#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <Windows.h>

#include "Transform.h"
#include "Patterns.h"

class Camera : public Singleton<Camera>
{
public:
	Camera();
	~Camera();

	DirectX::XMFLOAT4X4 ViewMatrix() const;
	DirectX::XMFLOAT4X4 ProjectionMatrix() const;
	
	void Update(float deltaTime);
	void Resize(unsigned int width, unsigned int height);

	float fov, nearPlane, farPlane;
	DirectX::BoundingFrustum frustum;
	TransformData::Handle transform;
	
private:
	DirectX::XMFLOAT4X4 viewMat, projMat;
	bool fpsEnabled;
	POINT fpsPos;
};