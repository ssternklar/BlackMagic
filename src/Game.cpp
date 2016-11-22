#include "Game.h"
#include "Vertex.h"

#include <DirectXCollision.h>
#include <iostream>
#include "Texture.h"
#include "TransformData.h"
#include "WICTextureLoader.h"

// For the DirectX Math library
using namespace DirectX;

using namespace ECS;

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

	// TODO: Pass our custom allocator here. Also see ECS.h line 2.
	gameWorld = ECS::World::createWorld();

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
	gameWorld->destroyWorld(); // no need to delete the world, this cleans up everything.

	delete _transformMemory;
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	_renderer = std::make_unique<GraphicsDevice>();
	
	auto hr = _renderer->InitDx(hWnd, width, height);
	if (hr)
	{
		std::cout << "[Error] Renderer init failed with code " << hr << std::endl;
		std::exit(hr);
	}
	
	_content = std::make_unique<ContentManager>(_renderer->Device(), _renderer->Context(), L"./assets/");
	_renderer->Init(_content.get());

	dxFeatureLevel = _renderer->FeatureLevel();

	LoadContent();

	_globalLight = DirectionalLight{
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ -1, -1, 0 }
	};
}


void Game::LoadContent()
{
	auto sphere = _content->Load<Mesh>(L"/models/sphere.obj");
	auto plane = _content->Load<Mesh>(L"/models/plane.obj");

	auto rocks = _content->Load<Texture>(L"/textures/rock.jpg");
	auto rocksNormals = _content->Load<Texture>(L"/textures/rockNormals.jpg");

	auto sand = _content->Load<Texture>(L"/textures/sand_texture.JPG");
	auto sandNormals = _content->Load<Texture>(L"textures/sand_normal.JPG");

	auto gPassVS = _content->Load<VertexShader>(L"/shaders/GBufferVS.cso");
	auto gPassPS = _content->Load<PixelShader>(L"/shaders/GBufferPS.cso");

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
		gPassVS, gPassPS,
		rocks, sampler,
		rocksNormals);

	auto testMat = std::make_shared<Material>(
		gPassVS, gPassPS,
		sand, sampler,
		sandNormals);
	
	XMFLOAT4 quatIdentity;
	DirectX::XMStoreFloat4(&quatIdentity, DirectX::XMQuaternionIdentity());
	XMFLOAT3 defaultScale = { 1, 1, 1 };

	for(size_t y = 5; y < 15; y++)
	{
		for(size_t x = 5; x < 15; x++)
		{
			auto pos = XMFLOAT3{ static_cast<float>(x)*1.5f, 0, static_cast<float>(y)*1.5f };
			Entity* ent = gameWorld->create();
			ent->assign<Transform>(pos, quatIdentity, defaultScale);
			ent->assign<Renderable>(sphere, gridMat);
			ent->assign<BoundingSphere>(pos, 10.0f);
		}
	}

	Entity* planeEntity = gameWorld->create();
	auto planePos = XMFLOAT3{ 15, -1.1f, 15 };
	auto planeScale = XMFLOAT3{ 15, 1, 15 };
	planeEntity->assign<Transform>(planePos, quatIdentity, planeScale);
	planeEntity->assign<Renderable>(plane, testMat);
	planeEntity->assign<BoundingBox>(planePos, planeScale);

	// Add our test system
	gameWorld->registerSystem(new TestSystem());
}

// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	if (_renderer && _renderer->Device())
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

	//_directionalLights[0].Direction.x = cos(totalTime*0.3f);
	//_directionalLights[0].Direction.y = sin(totalTime*0.3f);

	_camera.Update(deltaTime);

	// World update
	// this ticks all registered systems
	gameWorld->tick(deltaTime);
	
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
	std::vector<Entity*> renderables;
	renderables.reserve(100);
	_renderer->Cull(_camera, gameWorld, renderables);
	_renderer->Render(_camera, renderables, _globalLight);
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

ECS::World* Game::getGameWorld() const
{
	return gameWorld;
}

#pragma endregion
