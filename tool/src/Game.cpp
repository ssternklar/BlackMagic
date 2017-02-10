#include "Game.h"
#include "Vertex.h"

using namespace DirectX;

Game::Game(HINSTANCE hInstance)	: DXCore(hInstance, "tool", 1280, 720, true)
{
	vertexBuffer = 0;
	indexBuffer = 0;
	vertexShader = 0;
	pixelShader = 0;

#if defined(DEBUG) || defined(_DEBUG)
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.");
#endif
}

Game::~Game()
{
	if (vertexBuffer) { vertexBuffer->Release(); }
	if (indexBuffer) { indexBuffer->Release(); }

	delete vertexShader;
	delete pixelShader;
}

void Game::Init()
{
	LoadShaders();
	CreateMatrices();
	CreateBasicGeometry();
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Game::LoadShaders()
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

void Game::CreateMatrices()
{
	XMMATRIX W = XMMatrixIdentity();
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(W));
	XMVECTOR pos = XMVectorSet(0, 0, -5, 0);
	XMVECTOR dir = XMVectorSet(0, 0, 1, 0);
	XMVECTOR up  = XMVectorSet(0, 1, 0, 0);
	XMMATRIX V   = XMMatrixLookToLH(pos, dir, up);
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V));
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * 3.1415926535f, (float)width / height, 0.1f, 100.0f);
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P));
}

void Game::CreateBasicGeometry()
{
	XMFLOAT4 red	= XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green	= XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue	= XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

	Vertex vertices[] = 
	{
		{ XMFLOAT3(+0.0f, +1.0f, +0.0f), red },
		{ XMFLOAT3(+1.5f, -1.0f, +0.0f), blue },
		{ XMFLOAT3(-1.5f, -1.0f, +0.0f), green },
	};

	int indices[] = { 0, 1, 2 };

	D3D11_BUFFER_DESC vbd;
	vbd.Usage				= D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth			= sizeof(Vertex) * 3;
	vbd.BindFlags			= D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags		= 0;
	vbd.MiscFlags			= 0;
	vbd.StructureByteStride	= 0;

	D3D11_SUBRESOURCE_DATA initialVertexData;
	initialVertexData.pSysMem = vertices;

	device->CreateBuffer(&vbd, &initialVertexData, &vertexBuffer);

	D3D11_BUFFER_DESC ibd;
	ibd.Usage               = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth           = sizeof(int) * 3;
	ibd.BindFlags           = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags      = 0;
	ibd.MiscFlags           = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialIndexData;
	initialIndexData.pSysMem = indices;

	device->CreateBuffer(&ibd, &initialIndexData, &indexBuffer);
}

void Game::OnResize()
{
	DXCore::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * 3.1415926535f, (float)width / height, 0.1f, 100.0f);
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P));
}

void Game::Update(float deltaTime, float totalTime)
{
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();
}

void Game::Draw(float deltaTime, float totalTime)
{
	const float color[4] = {0.4f, 0.6f, 0.75f, 0.0f};
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

#pragma region Mouse Input
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	prevMousePos.x = x;
	prevMousePos.y = y;

	SetCapture(hWnd);
}

void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	ReleaseCapture();
}

void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	prevMousePos.x = x;
	prevMousePos.y = y;
}

void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	
}
#pragma endregion