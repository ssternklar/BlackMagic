#pragma once

#include "allocators\FixedBlockAllocator.h"
#include <DirectXMath.h>
#include <stack>
#include "Transform.h"

namespace BlackMagic
{
	class TransformData
	{
		friend class Transform;

	public:
		TransformData();
		size_t Size = sizeof(DirectX::XMFLOAT4X4*) + sizeof(DirectX::XMFLOAT4*) + 2 * sizeof(DirectX::XMFLOAT3*) + sizeof(bool*) + sizeof(TransformID);
		static TransformData* GetSingleton();

		TransformID AllocateTransform();
		void DeallocateTransform(TransformID);

		void UpdateTransforms();
		DirectX::XMFLOAT4X4* GetMatrix(TransformID id);
	private:
		FixedBlockAllocator matrixAllocator;
		DirectX::XMFLOAT4X4 _matrices[400];
		DirectX::XMFLOAT4 _rotations[400];
		DirectX::XMFLOAT3 _positions[400];
		DirectX::XMFLOAT3 _scales[400];
		size_t highestAllocated = 0;
		static TransformData* singletonRef;
	};
}