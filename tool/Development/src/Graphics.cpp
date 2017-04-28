#include "Graphics.h"
#include "Tool.h"
#include "Assets.h"

using namespace DirectX;

Graphics::Graphics()
{
	device = nullptr;
	context = nullptr;
	swapChain = nullptr;
	backBufferRTV = nullptr;
	depthStencilView = nullptr;
}

Graphics::~Graphics()
{
	if (depthStencilView) { depthStencilView->Release(); }
	if (backBufferRTV) { backBufferRTV->Release(); }

	if (swapChain) { swapChain->Release(); }
	if (context) { context->Release(); }
	if (device) { device->Release(); }
}

HRESULT Graphics::Init(HINSTANCE hInstance, unsigned int windowWidth, unsigned int windowHeight)
{
	width = windowWidth;
	height = windowHeight;

	HRESULT hr = S_OK;
	hr = InitWindow(hInstance);
	if (FAILED(hr)) return hr;
	hr = InitDirectX();
	if (FAILED(hr)) return hr;

	ShaderData<SimpleVertexShader>::Instance().Init(device, context);
	ShaderData<SimplePixelShader>::Instance().Init(device, context);
	
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	color = { 0.4f, 0.6f, 0.75f, 0.0f };

	return hr;
}

HRESULT Graphics::InitWindow(HINSTANCE hInstance)
{
	WNDCLASS wndClass = {};
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = Tool::WindowProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = "Direct3DWindowClass";

	if (!RegisterClass(&wndClass))
	{
		DWORD error = GetLastError();

		if (error != ERROR_CLASS_ALREADY_EXISTS)
			return HRESULT_FROM_WIN32(error);
	}

	RECT clientRect;
	SetRect(&clientRect, 0, 0, width, height);
	AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, false);

	RECT desktopRect;
	GetClientRect(GetDesktopWindow(), &desktopRect);
	int centeredX = (desktopRect.right / 2) - (clientRect.right / 2);
	int centeredY = (desktopRect.bottom / 2) - (clientRect.bottom / 2);

	hWnd = CreateWindow(
		wndClass.lpszClassName,
		"ChiroTool",
		WS_OVERLAPPEDWINDOW,
		centeredX,
		centeredY,
		clientRect.right - clientRect.left,
		clientRect.bottom - clientRect.top,
		0,
		0,
		hInstance,
		0);

	if (hWnd == NULL)
	{
		DWORD error = GetLastError();
		return HRESULT_FROM_WIN32(error);
	}

	ShowWindow(hWnd, SW_SHOW);

	return S_OK;
}

HRESULT Graphics::InitDirectX()
{
	unsigned int deviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 1;
	swapDesc.BufferDesc.Width = width;
	swapDesc.BufferDesc.Height = height;
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.Flags = 0;
	swapDesc.OutputWindow = hWnd;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Windowed = true;

	HRESULT hr = S_OK;

	hr = D3D11CreateDeviceAndSwapChain(
		0,
		D3D_DRIVER_TYPE_HARDWARE,
		0,
		deviceFlags,
		0,
		0,
		D3D11_SDK_VERSION,
		&swapDesc,
		&swapChain,
		&device,
		&dxFeatureLevel,
		&context);
	if (FAILED(hr)) return hr;

	ID3D11Texture2D* backBufferTexture;
	swapChain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		(void**)&backBufferTexture);

	device->CreateRenderTargetView(
		backBufferTexture,
		0,
		&backBufferRTV);
	backBufferTexture->Release();

	D3D11_TEXTURE2D_DESC depthStencilDesc = {};
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;

	ID3D11Texture2D* depthBufferTexture;
	device->CreateTexture2D(&depthStencilDesc, 0, &depthBufferTexture);
	device->CreateDepthStencilView(depthBufferTexture, 0, &depthStencilView);
	depthBufferTexture->Release();

	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);

	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	return S_OK;
}

void Graphics::Resize(unsigned int width, unsigned int height)
{
	if (!device)
		return;

	this->width = width;
	this->height = height;

	if (depthStencilView) { depthStencilView->Release(); }
	if (backBufferRTV) { backBufferRTV->Release(); }

	swapChain->ResizeBuffers(
		1,
		width,
		height,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		0);

	ID3D11Texture2D* backBufferTexture;
	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBufferTexture));
	device->CreateRenderTargetView(backBufferTexture, 0, &backBufferRTV);
	backBufferTexture->Release();

	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;

	ID3D11Texture2D* depthBufferTexture;
	device->CreateTexture2D(&depthStencilDesc, 0, &depthBufferTexture);
	device->CreateDepthStencilView(depthBufferTexture, 0, &depthStencilView);
	depthBufferTexture->Release();

	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);

	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);
}

void Graphics::Clear()
{
	context->ClearRenderTargetView(backBufferRTV, &color.x);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Graphics::Draw(float deltaTime)
{
	context->ClearRenderTargetView(backBufferRTV, &color.x);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	SceneData::Handle scene = SceneData::Instance().CurrentScene();

	if (!scene.ptr())
		return;
	
	UINT stride = sizeof(Mesh::Vertex);
	UINT offset = 0;
	EntityData::Handle entity;
	SimpleVertexShader* vertexShader;

	for (size_t i = 0; i < scene->entities.size(); ++i)
	{
		entity = scene->entities[i];
		vertexShader = entity->material->vertexShader->shader;

		vertexShader->SetMatrix4x4("world", scene->entities[i]->transform->matrix);
		vertexShader->SetMatrix4x4("view", Camera::Instance().ViewMatrix());
		vertexShader->SetMatrix4x4("projection", Camera::Instance().ProjectionMatrix());

		MaterialData::Instance().Use(entity->material);

		context->IASetVertexBuffers(0, 1, &scene->entities[i]->mesh->vertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(scene->entities[i]->mesh->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		context->DrawIndexed(scene->entities[i]->mesh->faceCount, 0, 0);
	}
}

void Graphics::Present()
{
	swapChain->Present(0, 0);
}

HWND Graphics::GetHandle()
{
	return hWnd;
}

ID3D11Device* Graphics::GetDevice()
{
	return device;
}

ID3D11DeviceContext* Graphics::GetContext()
{
	return context;
}

D3D_FEATURE_LEVEL Graphics::GetFeatureLevel()
{
	return dxFeatureLevel;
}

unsigned int Graphics::GetWidth()
{
	return width;
}

unsigned int Graphics::GetHeight()
{
	return height;
}
