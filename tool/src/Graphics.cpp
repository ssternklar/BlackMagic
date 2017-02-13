#include "Graphics.h"
#include "Vertex.h"
#include "Tool.h"

using namespace DirectX;

Graphics::Graphics(unsigned int windowWidth, unsigned int windowHeight)
{
	width = windowWidth;
	height = windowHeight;

	device = nullptr;
	context = nullptr;
	swapChain = nullptr;
	backBufferRTV = nullptr;
	depthStencilView = nullptr;

	vertexBuffer = nullptr;
	indexBuffer = nullptr;
	vertexShader = nullptr;
	pixelShader = nullptr;
}

Graphics::~Graphics()
{
	if (depthStencilView) { depthStencilView->Release(); }
	if (backBufferRTV) { backBufferRTV->Release(); }

	if (swapChain) { swapChain->Release(); }
	if (context) { context->Release(); }
	if (device) { device->Release(); }

	if (vertexBuffer) { vertexBuffer->Release(); }
	if (indexBuffer) { indexBuffer->Release(); }

	delete vertexShader;
	delete pixelShader;
}

HRESULT Graphics::Init(HINSTANCE hInstance, const char* windowTitle)
{
	HRESULT hr = S_OK;
	hr = InitWindow(hInstance, windowTitle);
	if (FAILED(hr)) return hr;
	hr = InitDirectX();
	if (FAILED(hr)) return hr;

	LoadShaders();
	CreateMatrices();
	CreateBasicGeometry();
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return hr;
}

HRESULT Graphics::InitWindow(HINSTANCE hInstance, const char* windowTitle)
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
		windowTitle,
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

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * 3.1415926535f, (float)width / height, 0.1f, 100.0f);
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P));
}

void Graphics::Draw(float deltaTime, float totalTime)
{
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	vertexShader->SetMatrix4x4("world", worldMatrix);
	vertexShader->SetMatrix4x4("view", viewMatrix);
	vertexShader->SetMatrix4x4("projection", projectionMatrix);
	vertexShader->CopyAllBufferData();
	vertexShader->SetShader();
	pixelShader->SetShader();
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	context->DrawIndexed(3, 0, 0);
	swapChain->Present(0, 0);
}

void Graphics::LoadShaders()
{
	wchar_t path[MAX_PATH];
	// temporary patch for Chris's code
#define LOAD_FAILED_SHADER(var, shader) \
do \
{ \
	wcscpy_s(path, L"shaders/"); \
	wcscat_s(path, shader); \
	wcscat_s(path, L".cso"); \
	var->LoadShaderFile(path); \
} while(0)

#ifdef _DEBUG
#define LOAD_SHADER(var, shader) \
do \
{ \
wcscpy_s(path, L"Debug/shaders/"); \
wcscat_s(path, shader); \
wcscat_s(path, L".cso"); \
if (!var->LoadShaderFile(path)) \
	{LOAD_FAILED_SHADER(var, shader);} \
}while(0)
#else
#define LOAD_SHADER(var, shader) \
do \
{ \
wcscpy_s(path, L"Release/shaders/"); \
wcscat_s(path, shader); \
wcscat_s(path, L".cso"); \
if (!var->LoadShaderFile(path)) \
	{LOAD_FAILED_SHADER(var, shader);} \
}while(0)
#endif

	vertexShader = new SimpleVertexShader(device, context);
	LOAD_SHADER(vertexShader, L"VertexShader");

	pixelShader = new SimplePixelShader(device, context);
	LOAD_SHADER(pixelShader, L"PixelShader");
}

void Graphics::CreateMatrices()
{
	XMMATRIX W = XMMatrixIdentity();
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(W));
	XMVECTOR pos = XMVectorSet(0, 0, -5, 0);
	XMVECTOR dir = XMVectorSet(0, 0, 1, 0);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX V = XMMatrixLookToLH(pos, dir, up);
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V));
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * 3.1415926535f, (float)width / height, 0.1f, 100.0f);
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P));
}

void Graphics::CreateBasicGeometry()
{
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

	Vertex vertices[] =
	{
		{ XMFLOAT3(+0.0f, +1.0f, +0.0f), red },
		{ XMFLOAT3(+1.5f, -1.0f, +0.0f), blue },
		{ XMFLOAT3(-1.5f, -1.0f, +0.0f), green },
	};

	int indices[] = { 0, 1, 2 };

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * 3;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialVertexData;
	initialVertexData.pSysMem = vertices;

	device->CreateBuffer(&vbd, &initialVertexData, &vertexBuffer);

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(int) * 3;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialIndexData;
	initialIndexData.pSysMem = indices;

	device->CreateBuffer(&ibd, &initialIndexData, &indexBuffer);
}

HWND Graphics::getHandle()
{
	return hWnd;
}

D3D_FEATURE_LEVEL Graphics::getFeatureLevel()
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
