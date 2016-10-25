#pragma once

#include <DirectXMath.h>
#include <memory>
#include <unordered_map>

#include "Camera.h"
#include "ContentManager.h"
#include "DirectionalLight.h"
#include "DXCore.h"
#include "Entity.h"
#include "GraphicsDevice.h"

class Game
	: public DXCore
{
public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize() override;
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

	// Overridden mouse input helper methods
	void OnMouseDown(WPARAM buttonState, int x, int y);
	void OnMouseUp(WPARAM buttonState, int x, int y);
	void OnMouseMove(WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta, int x, int y);
private:

	//Load meshes, textures, shaders, etc.
	void LoadContent();

	// The matrices to go from model space to screen space
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;

	//Store entities using unique_ptrs so they clean themselves up when they go out of scope
	std::vector<Entity> _entities;

	//Central storage for meshes so they get cleaned up properly
	//In a perfect world this would be the job of a resource cache class
	std::unordered_map<std::string, std::shared_ptr<Mesh>> _meshes;

	Camera _camera;

	std::vector<DirectionalLight> _directionalLights;
	
	std::unique_ptr<ContentManager> _content;
	std::unique_ptr<GraphicsDevice> _renderer;

	void* _transformMemory;
};
