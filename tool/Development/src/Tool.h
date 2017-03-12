#pragma once

#include "Scene.h"
#include "Graphics.h"

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
	void Update(float deltaTime);
	void OnResize(unsigned int width, unsigned int height);
	void CreateConsoleWindow(int bufferLines, int bufferColumns, int windowLines, int windowColumns);

	Graphics* graphics;
	Camera* camera;
	Scene scene;

	EntityHandle selectedEntity;

	bool statusInfo;
	bool entityEditor;

	bool resizing;
};

