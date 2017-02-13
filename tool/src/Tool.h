#pragma once

#include "Graphics.h"
#include "Camera.h"

class Tool
{
public:
	Tool(char* titleBarText, bool debugTitleBarStats);
	~Tool();

	static Tool* DXCoreInstance;
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT ProcessMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	HRESULT Run(HINSTANCE hInstance, unsigned int windowWidth, unsigned int windowHeight);
	void Quit();
	
private:
	Graphics* graphics;
	Camera* camera;

	void Update(float deltaTime, float totalTime);

	std::string windowTitle;
	bool titleBarStats;

	void CreateConsoleWindow(int bufferLines, int bufferColumns, int windowLines, int windowColumns);

	double perfCounterSeconds;
	float totalTime;
	float deltaTime;
	__int64 startTime;
	__int64 currentTime;
	__int64 previousTime;

	int fpsFrameCount;
	float fpsTimeElapsed;
	
	void UpdateTimer();
	void UpdateTitleBarStats();

	POINT prevMousePos;
};

