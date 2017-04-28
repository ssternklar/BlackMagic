#pragma once
#pragma comment(lib, "d3d11.lib")

#include <Windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

#include "Shader.h"
#include "Camera.h"
#include "Patterns.h"
#include "Texture.h"
#include "Mesh.h"

#define NUM_SHADOW_CASCADES 5
#define SHADOWMAP_DIM 1025

struct DirectionalLight
{
	DirectX::XMFLOAT4 AmbientColor;
	DirectX::XMFLOAT4 DiffuseColor;
	DirectX::XMFLOAT3 Direction;
	float padding;
	DirectX::XMFLOAT3 Up;
};

class Graphics : public Singleton<Graphics>
{
public:
	Graphics();
	~Graphics();

	HRESULT Init(HINSTANCE hInstance, unsigned int windowWidth, unsigned int windowHeight);
	void Resize(unsigned int width, unsigned int height);
	void Clear();
	void Render(float deltaTime);
	void Present();

	HWND GetHandle();
	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetContext();
	D3D_FEATURE_LEVEL GetFeatureLevel();
	unsigned int GetWidth();
	unsigned int GetHeight();

	DirectX::XMFLOAT4 color;

private:
	HRESULT InitWindow(HINSTANCE hInstance);
	HRESULT InitDirectX();

	HWND hWnd;

	D3D_FEATURE_LEVEL dxFeatureLevel;
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swapChain;
	ID3D11RenderTargetView* backBuffer;
	ID3D11DepthStencilView* depthStencil;

	ID3D11ShaderResourceView* depthStencilTexture;
	TextureData::Handle albedoMap;
	TextureData::Handle roughnessMap;
	TextureData::Handle metalMap;
	TextureData::Handle cavityMap;
	TextureData::Handle normalMap;
	TextureData::Handle positionMap;
	TextureData::Handle lightMap;
	VertexShaderData::Handle lightPassVS;
	PixelShaderData::Handle lightPassPS;
	VertexShaderData::Handle fxaaVS;
	PixelShaderData::Handle fxaaPS;
	PixelShaderData::Handle mergePS;
	ID3D11SamplerState* gBufferSampler;
	ID3D11Buffer* quad;
	TextureData::Handle cosLookup;
	ID3D11BlendState* blendState;

	//Shadow mapping
	VertexShaderData::Handle shadowMapVS;
	ID3D11RasterizerState* shadowRS;
	ID3D11SamplerState* shadowSampler;
	DirectX::XMFLOAT4X4 shadowMatrices[NUM_SHADOW_CASCADES];
	DirectX::XMFLOAT4X4 shadowViews[NUM_SHADOW_CASCADES];
	DirectX::XMFLOAT4X4 shadowProjections[NUM_SHADOW_CASCADES];
	ID3D11DepthStencilView* shadowMapDSVs[NUM_SHADOW_CASCADES];
	ID3D11ShaderResourceView* shadowMapSRV;

	//Skybox
	MeshData::Handle skybox;
	TextureData::Handle skyboxTex;
	TextureData::Handle skyboxRadiance;
	TextureData::Handle skyboxIrradiance;
	VertexShaderData::Handle skyboxVS;
	PixelShaderData::Handle skyboxPS;
	ID3D11SamplerState* skyboxSampler;
	ID3D11SamplerState* envSampler;
	ID3D11DepthStencilState* skyboxDS;
	ID3D11RasterizerState* skyboxRS;

	DirectionalLight light;

	unsigned int width;
	unsigned int height;

	void InitBuffers();
	void RenderShadowMaps();
	void RenderSkybox();
};