#pragma once
#include <memory>
#include <vector>
#include "GraphicsDevice.h"

namespace BlackMagic {
	class DirectXGraphicsDevice : public GraphicsDevice
	{
	public:
		DirectXGraphicsDevice();
		~DirectXGraphicsDevice();

		ID3D11Device* Device() const;
		ID3D11DeviceContext* Context() const;
		D3D_FEATURE_LEVEL FeatureLevel() const;
	
		HRESULT InitDx(HWND window, UINT width, UINT height);
		virtual void Clear(DirectX::XMFLOAT4 color) override;
		virtual void Init(ContentManager* content) override;
		virtual void OnResize(UINT width, UINT height) override;
		virtual void Present(UINT interval, UINT flags) override;

		std::shared_ptr<ID3D11SamplerState> CreateSamplerState(D3D11_SAMPLER_DESC& desc);
		virtual GraphicsBuffer CreateBuffer(GraphicsBuffer::BufferType desc, void* data, size_t bufferSize) override;
		virtual void ModifyBuffer(GraphicsBuffer& buffer, GraphicsBuffer::BufferType bufferType, void* newData, size_t newBufferSize) override;

		virtual void Render(const Camera& cam, const std::vector<ECS::Entity*>& objects, const std::vector<DirectionalLight>& lights) override;
	private:
		ID3D11Device* _device;
		ID3D11DeviceContext* _context;
		IDXGISwapChain* _swapChain;
		ID3D11RenderTargetView* _backBuffer;
		ID3D11DepthStencilView* _depthStencil;

		//Resources for deferred rendering	
		ID3D11ShaderResourceView* _depthStencilTexture;
		Texture* _diffuseMap;
		Texture* _specularMap;
		Texture* _normalMap; 
		Texture* _positionMap;
		std::shared_ptr<VertexShader> _lightPassVS;
		std::shared_ptr<PixelShader> _lightPassPS;
		std::shared_ptr<ID3D11SamplerState> _gBufferSampler;
		ID3D11Buffer* _quad;

		D3D_FEATURE_LEVEL _featureLevel;
		UINT _width, _height;

		void InitBuffers();
		Texture* createEmptyTexture(D3D11_TEXTURE2D_DESC& desc);

	};
}