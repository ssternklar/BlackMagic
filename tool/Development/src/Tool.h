#pragma once

#include "dear imgui\imgui.h"
#include "dear imgui\imgui_impl_dx11.h"

#include "Graphics.h"
#include "Entity.h"

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
	void invokeGUI();
	void OnResize(unsigned int width, unsigned int height);
	void CreateConsoleWindow(int bufferLines, int bufferColumns, int windowLines, int windowColumns);

	Graphics* graphics;
	Camera* camera;

	bool resizing;

	void ScanEntities(float x, float y);
	void SelectEntity(EntityData::Handle ent);
	EntityData::Handle selectedEntity;

	// imgui
	bool meshImporter = false;
	int meshIndex = -1;
};

