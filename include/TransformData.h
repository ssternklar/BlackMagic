#pragma once

#include <DirectXMath.h>
#include <stack>
#include "Transform.h"


class TransformData
{
	friend class Transform;

public:
	static size_t Size;

	static void Init(size_t allocCount, void* mem);

	static TransformID AllocateTransform();
	static void DeallocateTransform(TransformID);

	static void UpdateTransforms();
	static const DirectX::XMFLOAT4X4* GetMatrix(TransformID id);
private:
	static DirectX::XMFLOAT4X4 _matrices[100];
	static DirectX::XMFLOAT4 _rotations[100];
	static DirectX::XMFLOAT3 _positions[100];
	static DirectX::XMFLOAT3 _scales[100];
	static std::stack<TransformID> _availableTransforms;
	static TransformID _nextAvailableTransform;
};