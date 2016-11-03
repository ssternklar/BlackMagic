#pragma once
#include <memory>
#include <vector>

#include "Camera.h"
#include "Entity.h"
#include "Renderable.h"
#include "ContentManager.h"

class GraphicsDevice
{
public:
	GraphicsDevice();
	~GraphicsDevice();

	ID3D11Device* Device() const;
	ID3D11DeviceContext* Context() const;
	D3D_FEATURE_LEVEL FeatureLevel() const;

	void Clear(XMFLOAT4 color);
	HRESULT InitDx(HWND window, UINT width, UINT height);
	void Init(ContentManager* content);
	void OnResize(UINT width, UINT height);
	void Present(UINT interval, UINT flags);

	std::shared_ptr<ID3D11SamplerState> CreateSamplerState(D3D11_SAMPLER_DESC& desc);
	ID3D11Buffer* CreateBuffer(const D3D11_BUFFER_DESC& desc, const D3D11_SUBRESOURCE_DATA& data);

	void Cull(const Camera& cam, std::vector<Entity>& sceneObjects, std::vector<Renderable*>& objectsToDraw)
	{
		//TODO: Actually implement frustum culling
		for (auto& r : sceneObjects)
			objectsToDraw.push_back(&r);
	}

	void Render(const Camera& cam, const std::vector<Renderable*>& objects, const std::vector<DirectionalLight>& lights);
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
