#pragma once

#include "Camera.h"
#include <vector>
#include "ECS.h"

class Renderer
{
public:
	void Cull(const Camera& cam, std::vector<ECS::Entity*> objects);
	void Render(const Camera& cam, std::vector<ECS::Entity*> entities);
private:

};
