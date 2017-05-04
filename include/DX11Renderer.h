#pragma once
#ifdef BM_PLATFORM_WINDOWS
#pragma comment (lib, "d3d11.lib")
#include <d3d11.h>
#include <memory>
#include <vector>
#include "BMMath.h"
#include "Renderer.h"
#include "ContentManager.h"

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
		virtual void Clear(BlackMagic::Vector4 color) override;
		virtual void Init(ContentManager* content) override;
		virtual void OnResize(UINT width, UINT height) override;
		virtual void Present(UINT interval, UINT flags) override;

		virtual Sampler CreateSampler() override;
		Sampler CreateSampler(D3D11_SAMPLER_DESC desc);
		virtual Buffer CreateBuffer(Buffer::Type desc, void* data, size_t bufferSize) override;
		virtual void ModifyBuffer(Buffer& buffer, Buffer::Type bufferType, void* newData, size_t newBufferSize) override;
		virtual void Cull(const Camera& cam, const std::vector<Entity*> objects, std::vector<Entity*>& objectsToDraw, bool debugDrawEverything = false) override;
		virtual void Render(const Camera& cam, const std::vector<Entity*>& objects, const DirectionalLight& sceneLight) override;
		virtual void RenderSkybox(const Camera& cam) override;
		virtual Texture CreateTexture(BlackMagic::byte* data, size_t size, Texture::Type type, Texture::Usage usage) override;
		virtual Texture CreateTexture(const TextureDesc& desc) override;
		virtual void ReleaseResource(void* resource) override;
		virtual void AddResourceRef(void* resource) override;
		virtual GraphicsContext GetCurrentContext() override;
		virtual const char* GetShaderExtension() override;

	private:
		ComPtr<ID3D11Device> _device;
		ComPtr<ID3D11DeviceContext> _context;
		ComPtr<IDXGISwapChain> _swapChain;
		ComPtr<ID3D11RenderTargetView> _backBuffer;
		ComPtr<ID3D11DepthStencilView> _depthStencil;
		BestFitAllocator* contentManagerAllocator;

		//Resources for deferred rendering	
		ComPtr<ID3D11ShaderResourceView> _depthStencilTexture;
		Texture* _albedoMap;
		Texture* _roughnessMap;
		Texture* _metalMap;
		Texture* _cavityMap;
		Texture* _normalMap; 
		Texture* _positionMap;
		Texture* _lightMap;
		VertexShader* _lightPassVS;
		PixelShader* _lightPassPS;
		VertexShader* _fxaaVS;
		PixelShader* _fxaaPS;
		PixelShader* _mergePS;
		Sampler _gBufferSampler;
		ComPtr<ID3D11Buffer> _quad;
		Texture* _cosLookup;

		D3D_FEATURE_LEVEL _featureLevel;
		UINT _width, _height;

		ComPtr<ID3D11BlendState> _blendState;
		//Shadow mapping
		VertexShader* _shadowMapVS;
		ComPtr<ID3D11RasterizerState> _shadowRS;
		Sampler _shadowSampler;
		DirectX::XMFLOAT4X4 _shadowMatrices[NUM_SHADOW_CASCADES];
		DirectX::XMFLOAT4X4 _shadowViews[NUM_SHADOW_CASCADES];
		DirectX::XMFLOAT4X4 _shadowProjections[NUM_SHADOW_CASCADES];
		ComPtr<ID3D11DepthStencilView> _shadowMapDSVs[NUM_SHADOW_CASCADES];
		ComPtr<ID3D11ShaderResourceView> _shadowMapSRV;

		//Skybox
		Mesh* _skybox;
		Cubemap* _skyboxTex;
		Cubemap* _skyboxRadiance;
		Cubemap* _skyboxIrradiance;
		VertexShader* _skyboxVS;
		PixelShader* _skyboxPS;
		Sampler _skyboxSampler;
		Sampler _envSampler;
		ComPtr<ID3D11DepthStencilState> _skyboxDS;
		ComPtr<ID3D11RasterizerState> _skyboxRS;

		//Projected Textures
		PixelShader* _projectionPS;
		Sampler _projectionSampler;
		ComPtr<ID3D11BlendState> _projectionBlend;

        //HDR framebuffer
        Texture* _hdrBuffer;
        PixelShader* _tonemapPS;

		void InitBuffers();
		Texture* createEmptyTexture(D3D11_TEXTURE2D_DESC& desc);
		void RenderShadowMaps(const Camera& cam, const std::vector<Entity*>& objects, const DirectionalLight& sceneLight);
	};
}
#endif