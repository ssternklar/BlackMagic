#pragma once

#include <DirectXMath.h>
#include <memory>
#include <unordered_map>

#include "Camera.h"
#include "ContentManager.h"
#include "DirectionalLight.h"
#include "DXCore.h"
#include "GraphicsDevice.h"
#include "ECS.h"

class TestSystem : public ECS::EntitySystem
{
	virtual void tick(ECS::World* world, float deltaTime) override
	{
		world->each<Transform>([deltaTime](auto ent, auto transform) {
			//transform->Rotate({ 0, 1, 0 }, 3.14f / 20.f * deltaTime);
		});
	}
};

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

	ECS::World* getGameWorld() const;
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

	//Central storage for meshes so they get cleaned up properly
	//In a perfect world this would be the job of a resource cache class
	std::unordered_map<std::string, std::shared_ptr<Mesh>> _meshes;

	Camera _camera;

	std::vector<DirectionalLight> _directionalLights;
	
	std::unique_ptr<ContentManager> _content;
	std::unique_ptr<GraphicsDevice> _renderer;

	void* _transformMemory;

	// ECS Stuff
	ECS::World* gameWorld;
};
