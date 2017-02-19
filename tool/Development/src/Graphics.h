#pragma once
#pragma comment(lib, "d3d11.lib")

#include <Windows.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include "SimpleShader.h"
#include "Camera.h"
#include "Mesh.h"

class Graphics
{
public:
	Graphics(unsigned int windowWidth, unsigned int windowHeight);
	~Graphics();

	HRESULT Init(HINSTANCE hInstance);
	void Resize(unsigned int width, unsigned int height);
	void Draw(Camera* camera, float deltaTime);
	void Present();

	HWND getHandle();
	ID3D11Device* getDevice();
	ID3D11DeviceContext* getContext();
	D3D_FEATURE_LEVEL getFeatureLevel();
	unsigned int GetWidth();
	unsigned int GetHeight();

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
	DirectX::XMFLOAT4X4 worldMatrix; // transform, done
	MeshHandle model;

	void LoadShaders();
	void CreateMatrices();

	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;
};