#pragma once

#include <DirectXMath.h>
#include "Texture.h"
#include "SplineControlPoint.h"

namespace BlackMagic
{
	class Projector
	{
	public:
		Projector(SplineControlPoint cp, const std::shared_ptr<Texture>& tex);
		DirectX::XMFLOAT4X4 Matrix() const;
		GraphicsTexture Texture() const;

	private:
		DirectX::XMFLOAT4X4 _mat;
		std::shared_ptr<BlackMagic::Texture> _tex;
	};
}