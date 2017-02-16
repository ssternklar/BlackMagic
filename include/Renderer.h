#pragma once
#include "Camera.h"
#include "ContentManager.h"
#include "Entity.h"
#include "GraphicsTypes.h"
#include "Renderable.h"
#include "Projector.h"
#define NUM_SHADOW_CASCADES 5
#define SHADOWMAP_DIM 1025

namespace BlackMagic
{
	class Renderer
	{
	public:
		virtual ~Renderer() = default;
		virtual void Clear(DirectX::XMFLOAT4 color) = 0;
		virtual void Init(ContentManager* content) = 0;
		virtual void OnResize(unsigned int width, unsigned int height) = 0;
		virtual void Present(unsigned int interval, unsigned int flags) = 0;
		virtual GraphicsBuffer CreateBuffer(GraphicsBuffer::BufferType bufferType, void* data, size_t bufferSize) = 0;
		virtual void ModifyBuffer(GraphicsBuffer& buffer, GraphicsBuffer::BufferType bufferType, void* newData, size_t newBufferSize) = 0;
		virtual void CleanupBuffer(GraphicsBuffer buffer) = 0;
		virtual void Cull(const Camera& cam, const std::vector<Entity*> objects, std::vector<Entity*>& objectsToDraw, bool debugDrawEverything = false) = 0;
		virtual void Render(const Camera& cam, const std::vector<Entity*>& objects, const DirectionalLight& sceneLight) = 0;
		virtual void RenderSkybox(const Camera& cam) = 0;
		virtual GraphicsTexture CreateTexture(const wchar_t* texturePath, GraphicsTexture::TextureType type) = 0;
		virtual void ReleaseTexture(GraphicsTexture texture) = 0;
		virtual void ReleaseRenderTarget(GraphicsRenderTarget renderTarget) = 0;
		/*virtual GraphicsShader CreateShader(GraphicsShader::ShaderType shaderType, const char* shaderPath);
		virtual void CleanupShader(GraphicsShader::ShaderType shaderType, GraphicsShader shader);
		*/
	};
}
