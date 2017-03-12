#pragma once
#pragma comment(lib, "d3d11.lib")

#include <Windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

#include "SimpleShader.h"
#include "Camera.h"
#include "Entity.h"
#include "Shaders.h"

class Graphics
{
public:
	Graphics(unsigned int windowWidth, unsigned int windowHeight);
	~Graphics();

	HRESULT Init(HINSTANCE hInstance);
	void Resize(unsigned int width, unsigned int height);
	void Draw(Camera* camera, EntityData* entities, float deltaTime);
	void Present();

	HWND getHandle();
	ID3D11Device* getDevice();
	ID3D11DeviceContext* getContext();
	D3D_FEATURE_LEVEL getFeatureLevel();
	unsigned int GetWidth();
	unsigned int GetHeight();

	// temp
	void LoadShaders(ShaderData* shaders);

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
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;
};