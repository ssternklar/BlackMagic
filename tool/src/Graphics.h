#pragma once
#pragma comment(lib, "d3d11.lib")

#include <Windows.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include "SimpleShader.h"

class Graphics
{
public:
	Graphics(unsigned int windowWidth, unsigned int windowHeight);
	~Graphics();

	HRESULT Init(HINSTANCE hInstance, const char* windowTitle);
	void Resize(unsigned int width, unsigned int height);
	void Draw(float deltaTime, float totalTime);

	HWND getHandle();
	D3D_FEATURE_LEVEL getFeatureLevel();
	unsigned int GetWidth();
	unsigned int GetHeight();

private:
	HRESULT InitWindow(HINSTANCE hInstance, const char* windowTitle);
	HRESULT InitDirectX();

	HWND hWnd;

	D3D_FEATURE_LEVEL dxFeatureLevel;
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swapChain;

	ID3D11RenderTargetView* backBufferRTV;
	ID3D11DepthStencilView* depthStencilView;

	unsigned int width;
	unsigned int height;

	// temp
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	void LoadShaders();
	void CreateMatrices();
	void CreateBasicGeometry();

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;
};