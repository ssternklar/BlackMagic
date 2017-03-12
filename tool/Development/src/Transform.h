#pragma once

#include <DirectXMath.h>

#include "Singleton.h"
#include "PointerProxy.h"

struct Transform
{
	DirectX::XMFLOAT4 rot;
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 scale;
	DirectX::XMFLOAT4X4 matrix;
};

typedef proxy_ctr<Transform>::proxy_ptr TransformHandle;

class TransformData : public Singleton<TransformData>
{
public:
	TransformData();
	~TransformData();

	TransformHandle newTransform();
	void deleteTransform(TransformHandle handle);
	void UpdateTransforms();

	DirectX::XMFLOAT3 GetForward(TransformHandle handle);
	DirectX::XMFLOAT3 GetUp(TransformHandle handle);
	DirectX::XMFLOAT3 GetRight(TransformHandle handle);

	void Move(TransformHandle handle, DirectX::XMFLOAT3 delta);
	void Rotate(TransformHandle handle, DirectX::XMFLOAT4 quaternion);
	void Rotate(TransformHandle handle, DirectX::XMFLOAT3 axis, float angle);

private:
	ProxyVector<Transform> proxy;

	size_t numTransforms;
	size_t transformCount;
	Transform* transforms;
};