#include "Game.h"
#include "Vertex.h"

#include <new>
#include <iostream>
#include "Texture.h"
#include "TransformData.h"
#include "WICTextureLoader.h"

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance, // The application's handle
		"DirectX Game", // Text for the window's title bar
		1280, // Width of the window's client area
		720, // Height of the window's client area
		true),
	// Show extra stats (fps) in title bar?
	_camera({0, 0, -5}, {0, 0, 1}, 2)
{
	_camera.UpdateProjectionMatrix(width, height);
	_transformMemory = operator new(300 * TransformData::Size);
	TransformData::Init(300, _transformMemory);

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.");
#endif
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	delete _transformMemory;
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	_renderer = std::make_unique<GraphicsDevice>();
	
	auto hr = _renderer->Init(hWnd, width, height);
	if (hr)
	{
		std::cout << "[Error] Renderer init failed with code " << hr << std::endl;
		std::exit(hr);
	}
	
	_content = std::make_unique<ContentManager>(_renderer->Device(), _renderer->Context(), L"./assets/");

	dxFeatureLevel = _renderer->FeatureLevel();

	LoadContent();

	_directionalLights.push_back({
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 1, 0, 0 }
	});

	_directionalLights.push_back({
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ -1, 0, 0 }
	});
}


void Game::LoadContent()
{
	auto sphere = _content->Load<Mesh>(L"/models/sphere.obj");

	auto rocks = _content->Load<Texture>(L"/textures/rock.jpg");
	auto rocksNormals = _content->Load<Texture>(L"/textures/rockNormals.jpg");

	auto sand = _content->Load<Texture>(L"/textures/sand_texture.JPG");
	auto sandNormals = _content->Load<Texture>(L"textures/sand_normal.JPG");

	auto vertexShader = _content->Load<VertexShader>(L"/shaders/VertexShader.cso");
	auto pixelShader = _content->Load<PixelShader>(L"/shaders/PixelShader.cso");

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	auto sampler = _renderer->CreateSamplerState(samplerDesc);

	auto gridMat = std::make_shared<Material>(
		vertexShader, pixelShader,
		rocks, sampler,
		rocksNormals);
	
	XMFLOAT4 quatIdentity;
	DirectX::XMStoreFloat4(&quatIdentity, DirectX::XMQuaternionIdentity());
	XMFLOAT3 defaultScale = { 1, 1, 1 };

	for(size_t y = 0; y < 20; y++)
	{
		for(size_t x = 0; x < 20; x++)
		{
			_entities.emplace_back(Entity{ sphere, gridMat, XMFLOAT3{static_cast<float>(x), static_cast<float>(y), 0}, quatIdentity, defaultScale });
		}
	}

}

// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	if (_renderer && _renderer->Device() != nullptr)
		_renderer->OnResize(width, height);

	// Update the camera's projection matrix since the window size changed
	_camera.UpdateProjectionMatrix(width, height);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	_camera.Update(deltaTime);

	for(auto& e : _entities)
	{
		e.Update();
	}
	
	TransformData::UpdateTransforms();
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	//Cornflower blue
	const XMFLOAT4 color{0.4f, 0.6f, 0.75f, 0.0f};
	_renderer->Clear(color);
	std::vector<Renderable*> renderables;
	renderables.reserve(100);
	_renderer->Cull(_camera, _entities, renderables);
	_renderer->Render(_camera, renderables, _directionalLights);
	_renderer->Present(0, 0);
}

#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	// Caputure the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	SetCapture(hWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
// --------------------------------------------------------
void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
// --------------------------------------------------------
void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	//Only rotate the camera when the left mouse button is held
	if (buttonState & 0x1)
	{
		float dx = static_cast<float>(x - prevMousePos.x);
		float dy = static_cast<float>(y - prevMousePos.y);
		_camera.Rotate(dy * 0.002f * 3.14f, dx * 0.002f * 3.14f);
	}

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;
}

// --------------------------------------------------------
// Helper method for mouse wheel scrolling.  
// WheelDelta may be positive or negative, depending 
// on the direction of the scroll
// --------------------------------------------------------
void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	// Add any custom code here...
}
#pragma endregion
