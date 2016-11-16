#pragma once
#include <DirectXMath.h>
#include <DirectXCollision.h>

#define CAM_NEAR_Z 0.1f
#define CAM_FAR_Z 100.0f
#define CAM_FOV (0.4f*3.14f)

class Camera
{
public:
	Camera(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 dir, float speed);

	DirectX::XMFLOAT3 Position() const;
	DirectX::XMFLOAT3 Direction() const;
	DirectX::XMFLOAT4X4 ViewMatrix() const;
	DirectX::XMFLOAT4X4 ProjectionMatrix() const;
	const DirectX::BoundingFrustum& Frustum() const;

	void Rotate(float x, float y);
	void Update(float dt);

	//Updates this camera's projection matrix to match a new window size
	void UpdateProjectionMatrix(int width, int height);

private:
	DirectX::BoundingFrustum _frustum;
	DirectX::XMFLOAT4X4 _viewMat, _projMat;
	DirectX::XMFLOAT3 _pos, _dir;
	float _speed;
	DirectX::XMFLOAT2 _rot;
};
