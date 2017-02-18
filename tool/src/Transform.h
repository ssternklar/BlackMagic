#pragma once

#include <DirectXMath.h>
#include "PointerProxy.h"

struct Transform
{
	DirectX::XMFLOAT4 rot;
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 scale;
};

typedef proxy_ctr<Transform>::proxy_ptr TransformHandle;

class TransformData
{
public:
	static void Init();
	static TransformData* ptr;
	void ShutDown();

	TransformHandle newTransform();
	void deleteTransform(TransformHandle handle);
	void UpdateTransforms();

	const DirectX::XMFLOAT4X4* GetMatrix(TransformHandle handle);
	DirectX::XMFLOAT3 GetForward(TransformHandle handle);
	DirectX::XMFLOAT3 GetUp(TransformHandle handle);
	DirectX::XMFLOAT3 GetRight(TransformHandle handle);

	void Move(TransformHandle handle, DirectX::XMFLOAT3 delta);
	void Rotate(TransformHandle handle, DirectX::XMFLOAT4 quaternion);
	void Rotate(TransformHandle handle, DirectX::XMFLOAT3 axis, float angle);

private:
	TransformData();
	~TransformData();

	ProxyVector<Transform> proxy;

	TransformData(TransformData const&) = delete;
	void operator=(TransformData const&) = delete;

	size_t numTransforms;
	size_t transformCount;
	DirectX::XMFLOAT4X4* matrices;
	Transform* transforms;
};