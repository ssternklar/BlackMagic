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
		explicit Texture(BlackMagic::Renderer* device, BlackMagic::GraphicsTexture srView, BlackMagic::GraphicsRenderTarget rtView);
		~Texture();

		BlackMagic::GraphicsTexture GetGraphicsTexture() const;
		BlackMagic::GraphicsRenderTarget GetGraphicsRenderTarget() const;

	protected:
		BlackMagic::GraphicsTexture _srView;
		BlackMagic::GraphicsRenderTarget _rtView;
	};

	class Cubemap : public Texture
	{
	public:
		explicit Cubemap(BlackMagic::Renderer* device, BlackMagic::GraphicsTexture srView, BlackMagic::GraphicsRenderTarget rtView)
			: Texture(device, srView, rtView) {}
	};
}