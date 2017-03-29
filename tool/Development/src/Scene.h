#pragma once

#include <vector>

#include "Patterns.h"
#include "Entity.h"
#include "Tool.h"

struct Scene
{
	std::vector<EntityData::Handle> entities;
	EntityData::Handle selectedEntity;
};

class SceneData : public ProxyHandler<Scene, SceneData>
{
public:
	void Init(GuiData::EntityEditorData* gui);

	Handle Create(std::string name);
	Handle Get(std::string name);
	void Revoke(Handle handle);

	Handle LoadScene(std::string scenePath);
	void SaveScene(Handle handle);
	
	Handle CurrentScene();
	void SwapScene(size_t index);
	void SelectEntity(EntityData::Handle entity);

	const std::string root = "assets/scenes/";

private:
	Handle activeScene;
	GuiData::EntityEditorData* gui;
};
