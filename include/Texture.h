#pragma once

#include <d3d11.h>
#include <memory>
#include "GraphicsTypes.h"
#include "IResource.h"
namespace BlackMagic
{
	class Texture : public IResource
	{
	public:
		explicit Texture(BlackMagic::GraphicsDevice* device, BlackMagic::GraphicsTexture srView, BlackMagic::GraphicsRenderTarget rtView);
		~Texture();

		BlackMagic::GraphicsTexture GetGraphicsTexture() const;
		BlackMagic::GraphicsRenderTarget GetGraphicsRenderTarget() const;

	private:
		BlackMagic::GraphicsTexture _srView;
		BlackMagic::GraphicsRenderTarget _rtView;
	};
}