#pragma once
#pragma comment (lib, "d3d11.lib")
#include <d3d11.h>
#include <memory>
#include <vector>
#include "Renderer.h"
#include "Projector.h"

namespace BlackMagic {
	class DX11Renderer : public Renderer
	{
	public:
		DX11Renderer();
		~DX11Renderer();

		ComPtr<ID3D11Device> Device() const;
		ComPtr<ID3D11DeviceContext> Context() const;
		D3D_FEATURE_LEVEL FeatureLevel() const;
	
		HRESULT InitDx(HWND window, UINT width, UINT height);
		virtual void Clear(DirectX::XMFLOAT4 color) override;
		virtual void Init(ContentManager* content) override;
		virtual void OnResize(UINT width, UINT height) override;
		virtual void Present(UINT interval, UINT flags) override;

		ComPtr<ID3D11SamplerState> CreateSamplerState(D3D11_SAMPLER_DESC& desc);
		virtual GraphicsBuffer CreateBuffer(GraphicsBuffer::BufferType desc, void* data, size_t bufferSize) override;
		virtual void ModifyBuffer(GraphicsBuffer& buffer, GraphicsBuffer::BufferType bufferType, void* newData, size_t newBufferSize) override;
		virtual void CleanupBuffer(GraphicsBuffer buffer);
		virtual void Render(const Camera& cam, const std::vector<ECS::Entity*>& objects, const DirectionalLight& sceneLight) override;
		virtual void RenderSkybox(const Camera& cam) override;
		virtual void Cull(const Camera& cam, ECS::World* gameWorld, std::vector<ECS::Entity*>& objectsToDraw, bool debugDrawEverything = false) override;
		virtual GraphicsTexture CreateTexture(const wchar_t* texturePath, GraphicsTexture::TextureType type) override;
		virtual void ReleaseTexture(GraphicsTexture texture) override;
		virtual void ReleaseRenderTarget(GraphicsRenderTarget renderTarget) override;
	private:
		ComPtr<ID3D11Device> _device;
		ComPtr<ID3D11DeviceContext> _context;
		ComPtr<IDXGISwapChain> _swapChain;
		ComPtr<ID3D11RenderTargetView> _backBuffer;
		ComPtr<ID3D11DepthStencilView> _depthStencil;
		BestFitAllocator* contentManagerAllocator;

		//Resources for deferred rendering	
		ComPtr<ID3D11ShaderResourceView> _depthStencilTexture;
		Texture* _diffuseMap;
		Texture* _specularMap;
		Texture* _normalMap; 
		Texture* _positionMap;
		Texture* _lightMap;
		std::shared_ptr<VertexShader> _lightPassVS;
		std::shared_ptr<PixelShader> _lightPassPS;
		std::shared_ptr<VertexShader> _fxaaVS;
		std::shared_ptr<PixelShader> _fxaaPS;
		ComPtr<ID3D11SamplerState> _gBufferSampler;
		ComPtr<ID3D11Buffer> _quad;

		D3D_FEATURE_LEVEL _featureLevel;
		UINT _width, _height;

		ComPtr<ID3D11BlendState> _blendState;
		//Shadow mapping
		std::shared_ptr<VertexShader> _shadowMapVS;
		ComPtr<ID3D11RasterizerState> _shadowRS;
		ComPtr<ID3D11SamplerState>_shadowSampler;
		DirectX::XMFLOAT4X4 _shadowMatrices[NUM_SHADOW_CASCADES];
		DirectX::XMFLOAT4X4 _shadowViews[NUM_SHADOW_CASCADES];
		DirectX::XMFLOAT4X4 _shadowProjections[NUM_SHADOW_CASCADES];
		ComPtr<ID3D11DepthStencilView> _shadowMapDSVs[NUM_SHADOW_CASCADES];
		ComPtr<ID3D11ShaderResourceView> _shadowMapSRV;

		//Skybox
		std::shared_ptr<Mesh> _skybox;
		std::shared_ptr<Cubemap> _skyboxTex;
		std::shared_ptr<VertexShader> _skyboxVS;
		std::shared_ptr<PixelShader> _skyboxPS;
		ComPtr<ID3D11SamplerState> _skyboxSampler;
		ComPtr<ID3D11DepthStencilState> _skyboxDS;
		ComPtr<ID3D11RasterizerState> _skyboxRS;

		//Projected Textures
		std::shared_ptr<PixelShader> _projectionPS;
		ComPtr<ID3D11SamplerState> _projectionSampler;
		ComPtr<ID3D11BlendState> _projectionBlend;

		void InitBuffers();
		Texture* createEmptyTexture(D3D11_TEXTURE2D_DESC& desc);
		void RenderShadowMaps(const Camera& cam, const std::vector<ECS::Entity*>& objects, const DirectionalLight& sceneLight);
	};
}