#pragma once

#include <d3d11.h>
#include <memory>
#include "GraphicsTypes.h"
#include "Resource.h"

namespace BlackMagic
{
	class Texture : public Resource
	{
	public:
		enum Type
		{
			FLAT,
			CUBEMAP
		};

		enum Usage
		{
			READ,
			WRITE
		};

		explicit Texture(BlackMagic::Renderer* device, ResourceHandle* tex, ShaderResource* srView, RenderTarget* rtView);
		Texture(const Texture& t);
		~Texture();

		ShaderResource* GetShaderResource() const;
		RenderTarget* GetRenderTarget() const;
		
		Texture& operator=(const Texture& t);
		operator ShaderResource*() { return _srView; }
		operator RenderTarget*() { return _rtView; }

	protected:
		ShaderResource* _srView;
		RenderTarget* _rtView;
	};

	class Cubemap : public Texture
	{
	public:
		explicit Cubemap(BlackMagic::Renderer* device, ResourceHandle* tex, ShaderResource* srView, RenderTarget* rtView)
			: Texture(device, tex, srView, rtView) {}
		explicit Cubemap(const Texture& t) : Texture(t) {}
	};
}