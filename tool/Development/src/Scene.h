#pragma once

#include <vector>

#include "Patterns.h"
#include "Entity.h"
#include "Tool.h"

struct Scene
{
	std::vector<EntityData::Handle> entities;
	EntityData::Handle selectedEntity;
	bool willExport;
	Scene& operator=(Scene&& other)
	{
		entities = other.entities;
		selectedEntity = other.selectedEntity;
		willExport = other.willExport;
		return *this;
	}
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
	void Export(std::string path, Handle handle);
	
	Handle CurrentScene();
	void SwapScene(size_t index);
	void SelectEntity(EntityData::Handle entity);

	const std::string root = "assets/scenes/";
	std::vector<Handle> sceneExportConfig;

private:
	Handle activeScene;
	GuiData::EntityEditorData* gui;
};
