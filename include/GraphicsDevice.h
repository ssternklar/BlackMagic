#pragma once
#include <memory>
#include <vector>

#include "Camera.h"
#include "ECS.h"
#include "Renderable.h"
#include "ContentManager.h"

#define NUM_SHADOW_CASCADES 5
#define SHADOWMAP_DIM 1025

class GraphicsDevice
{
public:
	GraphicsDevice();
	~GraphicsDevice();

	ID3D11Device* Device() const;
	ID3D11DeviceContext* Context() const;
	D3D_FEATURE_LEVEL FeatureLevel() const;

	void Clear(DirectX::XMFLOAT4 color);
	HRESULT InitDx(HWND window, UINT width, UINT height);
	void Init(ContentManager* content);
	void OnResize(UINT width, UINT height);
	void Present(UINT interval, UINT flags);

	std::shared_ptr<ID3D11SamplerState> CreateSamplerState(D3D11_SAMPLER_DESC& desc);
	ID3D11Buffer* CreateBuffer(const D3D11_BUFFER_DESC& desc, const D3D11_SUBRESOURCE_DATA& data);

	// TODO: Add correct allocator type to vectors?
	void Cull(const Camera& cam, ECS::World* gameWorld, std::vector<ECS::Entity*>& objectsToDraw, bool debugDrawEverything = false);
	void Render(const Camera& cam, const std::vector<ECS::Entity*>& objects, const DirectionalLight& sceneLight);
private:
	ID3D11Device* _device;
	ID3D11DeviceContext* _context;
	IDXGISwapChain* _swapChain;
	ID3D11RenderTargetView* _backBuffer;
	ID3D11DepthStencilView* _depthStencil;
	ID3D11BlendState* _blendState;

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

	//Shadow mapping
	std::shared_ptr<VertexShader> _shadowMapVS;
	ID3D11RasterizerState* _shadowRS;
	std::shared_ptr<ID3D11SamplerState> _shadowSampler;
	DirectX::XMFLOAT4X4 _shadowMatrices[NUM_SHADOW_CASCADES];
	DirectX::XMFLOAT4X4 _shadowViews[NUM_SHADOW_CASCADES];
	DirectX::XMFLOAT4X4 _shadowProjections[NUM_SHADOW_CASCADES];
	//+1 since we're also storing the entire array's DSV/SRV
	ID3D11DepthStencilView* _shadowMapDSVs[NUM_SHADOW_CASCADES];
	ID3D11ShaderResourceView* _shadowMapSRV;


	D3D_FEATURE_LEVEL _featureLevel;
	UINT _width, _height;

	void InitBuffers();
	Texture* CreateEmptyTexture(D3D11_TEXTURE2D_DESC& desc);
	void RenderShadowMaps(const Camera& cam, const std::vector<ECS::Entity*>& objects, const DirectionalLight& sceneLight);

};
