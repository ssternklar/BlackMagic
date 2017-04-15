#pragma once

#include "dear imgui\imgui.h"
#include "dear imgui\imgui_impl_dx11.h"

#include "Graphics.h"

struct GuiData
{
	struct EntityEditorData
	{
		int meshIndex;
	}entityData;
	bool meshImporter;
	bool textureImporter;
	bool shaderImporter;
	int shaderType;
	bool sceneCreate;
	int sceneIndex;
	bool exitTool;
	struct ExporterData
	{
		bool prompt;
		int sceneCount;
		int dragIndex;
	}exportData;
};

class Tool
{
public:
	Tool();
	~Tool();

	static Tool* DXCoreInstance;
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT ProcessMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	HRESULT Run(HINSTANCE hInstance, unsigned int windowWidth, unsigned int windowHeight);
	void Quit();
	
private:
	void OnResize(unsigned int width, unsigned int height);
	void CreateConsoleWindow(int bufferLines, int bufferColumns, int windowLines, int windowColumns);
	
	void ScanEntities(float x, float y);

	bool resizing;

	// dear ImGui
	void HelloGUI();
	void ExitToolGUI();
	void InvokeGUI();
	void PromptImport();
	void PromptExport();
	GuiData gui;
};

template<typename T>
bool ComboAssetNames(void* data, int idx, const char** out_text)
{
	if (idx < 0 || idx >= (int)AssetManager::Instance().GetAssetCount<T>())
		return false;
	*out_text = AssetManager::Instance().GetAsset<T>(idx).name.c_str();
	return true;
}
