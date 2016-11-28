#pragma once
#include "Camera.h"
#include "ECS.h"
#include "Renderable.h"
#include "ContentManager.h"
#include "GraphicsBuffer.h"
namespace BlackMagic
{
	class GraphicsDevice
	{
	public:
		virtual void Clear(DirectX::XMFLOAT4 color) = 0;
		virtual void Init(ContentManager* content) = 0;
		virtual void OnResize(unsigned int width, unsigned int height) = 0;
		virtual void Present(unsigned int interval, unsigned int flags) = 0;
		virtual GraphicsBuffer CreateBuffer(GraphicsBuffer::BufferType bufferType, void* data, size_t bufferSize) = 0;
		virtual void ModifyBuffer(GraphicsBuffer& buffer, GraphicsBuffer::BufferType bufferType, void* newData, size_t newBufferSize) = 0;
		virtual void CleanupBuffer(GraphicsBuffer buffer) = 0;
		virtual void Render(const Camera& cam, const std::vector<ECS::Entity*>& objects, const std::vector<DirectionalLight>& lights) = 0;

		// TODO: Add correct allocator type to vectors?
		void Cull(const Camera& cam, ECS::World* gameWorld, std::vector<ECS::Entity*>& objectsToDraw)
		{
			for (auto* ent : gameWorld->each<Transform, Renderable>())
			{
				// TODO: Actual frustum culling
				objectsToDraw.push_back(ent);
			}
		}
	};
}