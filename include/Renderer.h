#pragma once
#include <type_traits>
#include <vector>

#include "Camera.h"
#include "Entity.h"
#include "Renderable.h"

class Renderer
{
public:
	unsigned int UseNormalMap = 0;
	Renderer(ID3D11Device* device, ID3D11DeviceContext* ctx);

	void Cull(const Camera& cam, std::vector<Entity>& sceneObjects, std::vector<Renderable*>& objectsToDraw)
	{
		//TODO: Actually implement frustum culling
		for (auto& r : sceneObjects)
			objectsToDraw.push_back(&r);
	}

	void Render(const Camera& cam, const std::vector<Renderable*>& objects, const std::vector<DirectionalLight> lights);
private:
	ID3D11Device* _device;
	ID3D11DeviceContext* _context;
};
