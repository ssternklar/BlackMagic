#pragma once

#include <DirectXMath.h>

typedef size_t TransformID;

struct Transform
{
	DirectX::XMFLOAT4 rot;
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 scale;
};

class TransformData
{
public:
	static void Init(size_t numTransforms);
	static TransformData* ptr;
	void ShutDown();

	TransformID newTransform();
	void deleteTransform(TransformID id);
	void UpdateTransforms();


	const DirectX::XMFLOAT4X4* GetMatrix(TransformID id);
	DirectX::XMFLOAT3 GetPosition(TransformID id);
	DirectX::XMFLOAT3 GetScale(TransformID id);
	DirectX::XMFLOAT4 GetRotation(TransformID id);
	DirectX::XMFLOAT3 GetForward(TransformID id);

	void Move(TransformID id, DirectX::XMFLOAT3 dp);
	void MoveTo(TransformID id, DirectX::XMFLOAT3 pos);
	void Rotate(TransformID id, DirectX::XMFLOAT4 quaternion);
	void Rotate(TransformID id, DirectX::XMFLOAT3 axis, float angle);
	void SetRotation(TransformID id, DirectX::XMFLOAT4 quaternion);
	void SetScale(TransformID id, DirectX::XMFLOAT3 scale);
private:
	TransformData(size_t numTransforms);
	~TransformData();

	TransformData(TransformData const&) = delete;
	void operator=(TransformData const&) = delete;

	size_t numTransforms;
	DirectX::XMFLOAT4X4* matrices;
	Transform* transforms;
	
	// 0x1 = used
	// 0x2 = dirty
	char* metaData;
	TransformID highestID = 0;
	TransformID nextFreeID = 0;
};