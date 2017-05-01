#pragma once

#include <vector>

#include "Patterns.h"
#include "Entity.h"
#include "Tool.h"
#include "Misc.h"

struct Scene
{
	std::vector<EntityData::Handle> entities;
	std::vector<MiscData::Handle> misc;
	EntityData::Handle selectedEntity;
	bool willExport;
};

class SceneData : public ProxyHandler<Scene, SceneData>
{
public:
	void Init(GuiData::EntityEditorData* gui);

	Handle Create(std::string name);
	Handle Get(std::string name);
	void Revoke(Handle handle);

	Handle Load(std::string scenePath);
	void Save(Handle handle);
	std::vector<uint16_t> Export(std::string path, Handle handle);
	
	Handle CurrentScene();
	void SwapScene(size_t index);
	void SelectEntity(EntityData::Handle entity);

	const std::string root = "assets/scenes/";
	std::vector<Handle> sceneExportConfig;

private:
	Handle activeScene;
	GuiData::EntityEditorData* gui;
};
