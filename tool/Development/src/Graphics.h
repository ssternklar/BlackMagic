#pragma once
#pragma comment(lib, "d3d11.lib")

#include <Windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

#include "SimpleShader.h"
#include "Camera.h"

class Graphics
{
public:
	Graphics(unsigned int windowWidth, unsigned int windowHeight);
	~Graphics();

	HRESULT Init(HINSTANCE hInstance);
	void Resize(unsigned int width, unsigned int height);
	void Clear();
	void Draw(Camera* camera, float deltaTime);
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

	ID3D11RenderTargetView* backBufferRTV;
	ID3D11DepthStencilView* depthStencilView;

	unsigned int width;
	unsigned int height;

	// temp
	void LoadShaders();
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;
};