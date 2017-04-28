#pragma once

#include "allocators\FixedBlockAllocator.h"
#include <BMMath.h>
#include <stack>
#include "Transform.h"

namespace BlackMagic
{
	class TransformData
	{
		friend class ::Transform;

	public:
		TransformData();
		size_t Size = sizeof(BlackMagic::Mat4*) + sizeof(BlackMagic::Vector4*) + 2 * sizeof(BlackMagic::Vector3*) + sizeof(bool*) + sizeof(TransformID);
		static TransformData* GetSingleton();

		TransformID AllocateTransform();
		void DeallocateTransform(TransformID);

		void UpdateTransforms();
		BlackMagic::Mat4* GetMatrix(TransformID id);
	private:
		FixedBlockAllocator matrixAllocator;
		BlackMagic::Mat4 _matrices[400];
		BlackMagic::Quaternion _rotations[400];
		BlackMagic::Vector3 _positions[400];
		BlackMagic::Vector3 _scales[400];
		size_t highestAllocated = 0;
		static TransformData* singletonRef;
	};
}