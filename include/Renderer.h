#pragma once
#include "Camera.h"
#include "Entity.h"
#include "GraphicsTypes.h"
#include "Renderable.h"
#include "Projector.h"
#include "BMMath.h"
#define NUM_SHADOW_CASCADES 5
#define SHADOWMAP_DIM 1025

namespace BlackMagic
{
	class ContentManager;

	class Renderer
	{
	public:
		virtual ~Renderer() = default;
		virtual void Clear(BlackMagic::Vector4 color) = 0;
		virtual void Init(ContentManager* content) = 0;
		virtual void OnResize(unsigned int width, unsigned int height) = 0;
		virtual void Present(unsigned int interval, unsigned int flags) = 0;
		virtual Buffer CreateBuffer(Buffer::Type bufferType, void* data, size_t bufferSize) = 0;
		virtual void ModifyBuffer(Buffer& buffer, Buffer::Type bufferType, void* newData, size_t newBufferSize) = 0;
		virtual void Cull(const Camera& cam, const std::vector<Entity*> objects, std::vector<Entity*>& objectsToDraw, bool debugDrawEverything = false) = 0;
		virtual void Render(const Camera& cam, const std::vector<Entity*>& objects, const DirectionalLight& sceneLight) = 0;
		virtual void RenderSkybox(const Camera& cam) = 0;
		virtual Texture CreateTexture(BlackMagic::byte* data, size_t size, Texture::Type type, Texture::Usage usage) = 0;
		virtual Texture CreateTexture(const TextureDesc& desc) = 0;
		virtual void AddResourceRef(void*) = 0;
		virtual void ReleaseResource(void*) = 0;
		virtual Sampler CreateSampler() = 0;
		virtual BestFitAllocator* GetCPUAllocator() = 0;
		virtual BestFitAllocator* GetGPUAllocator() = 0;
		virtual GraphicsContext* GetCurrentContext() = 0;
		/*virtual GraphicsShader CreateShader(GraphicsShader::ShaderType shaderType, const char* shaderPath);
		virtual void CleanupShader(GraphicsShader::ShaderType shaderType, GraphicsShader shader);
		*/
	};
}
