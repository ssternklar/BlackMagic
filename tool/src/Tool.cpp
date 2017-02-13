#include <WindowsX.h>
#include <sstream>

#include "Tool.h"
#include "Input.h"
#include "Transform.h"

using namespace DirectX;

Tool* Tool::DXCoreInstance = 0;

LRESULT Tool::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (DXCoreInstance)
		return DXCoreInstance->ProcessMessage(hWnd, uMsg, wParam, lParam);
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

Tool::Tool(char* titleBarText, bool debugTitleBarStats)
{
	DXCoreInstance = this;

	windowTitle = titleBarText;
	titleBarStats = debugTitleBarStats;

	fpsFrameCount = 0;
	fpsTimeElapsed = 0.0f;

	__int64 perfFreq;
	QueryPerformanceFrequency((LARGE_INTEGER*)&perfFreq);
	perfCounterSeconds = 1.0 / (double)perfFreq;

#if defined(DEBUG) || defined(_DEBUG)
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif
}

Tool::~Tool()
{
	delete graphics;
	TransformData::ptr->ShutDown();
}

HRESULT Tool::Run(HINSTANCE hInstance, unsigned int windowWidth, unsigned int windowHeight)
{
	__int64 now;
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	startTime = now;
	currentTime = now;
	previousTime = now;

	graphics = new Graphics(windowWidth, windowHeight);

	HRESULT hr = graphics->Init(hInstance, windowTitle.c_str());
	if (FAILED(hr)) return hr;

	Input::bindToControl("Quit", VK_ESCAPE);

	TransformData::Init(400);
	Input::bindToControl("new", 'N');
	Input::bindToControl("count", 'C');
	Input::bindToControl("delete", 'D');

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			UpdateTimer();
			if(titleBarStats)
				UpdateTitleBarStats();

			Update(deltaTime, totalTime);
			graphics->Draw(deltaTime, totalTime);
			Input::updateControlStates();
		}
	}

	return msg.wParam;
}

void Tool::Update(float deltaTime, float totalTime)
{
	if (Input::wasControlPressed("Quit"))
		Quit();

	if (Input::wasControlPressed("new"))
		TransformData::ptr->newTransform();

	if (Input::wasControlPressed("count"))
		++d;

	if (Input::wasControlPressed("delete"))
	{
		TransformData::ptr->deleteTransform(d);
		d = 0;
	}
}

void Tool::Quit()
{
	PostQuitMessage(0);
}

void Tool::UpdateTimer()
{
	__int64 now;
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	currentTime = now;

	deltaTime = max((float)((currentTime - previousTime) * perfCounterSeconds), 0.0f);

	totalTime = (float)((currentTime - startTime) * perfCounterSeconds);

	previousTime = currentTime;
}

void Tool::UpdateTitleBarStats()
{
	fpsFrameCount++;

	float timeDiff = totalTime - fpsTimeElapsed;
	if (timeDiff < 1.0f)
		return;

	float mspf = 1000.0f / (float)fpsFrameCount;

	std::ostringstream output;
	output.precision(6);
	output << windowTitle <<
		"    Width: "		<< graphics->GetWidth() <<
		"    Height: "		<< graphics->GetHeight() <<
		"    FPS: "			<< fpsFrameCount <<
		"    Frame Time: "	<< mspf << "ms";

	switch (graphics->getFeatureLevel())
	{
	case D3D_FEATURE_LEVEL_11_1: output << "    DX 11.1"; break;
	case D3D_FEATURE_LEVEL_11_0: output << "    DX 11.0"; break;
	case D3D_FEATURE_LEVEL_10_1: output << "    DX 10.1"; break;
	case D3D_FEATURE_LEVEL_10_0: output << "    DX 10.0"; break;
	case D3D_FEATURE_LEVEL_9_3:  output << "    DX 9.3";  break;
	case D3D_FEATURE_LEVEL_9_2:  output << "    DX 9.2";  break;
	case D3D_FEATURE_LEVEL_9_1:  output << "    DX 9.1";  break;
	default:                     output << "    DX ???";  break;
	}

	SetWindowText(graphics->getHandle(), output.str().c_str());
	fpsFrameCount = 0;
	fpsTimeElapsed += 1.0f;
}

void Tool::CreateConsoleWindow(int bufferLines, int bufferColumns, int windowLines, int windowColumns)
{
	CONSOLE_SCREEN_BUFFER_INFO coninfo;

	AllocConsole();
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = bufferLines;
	coninfo.dwSize.X = bufferColumns;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

	SMALL_RECT rect;
	rect.Left = 0;
	rect.Top = 0;
	rect.Right = windowColumns;
	rect.Bottom = windowLines;
	SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &rect);

	FILE *stream;
	freopen_s(&stream, "CONIN$", "r", stdin);
	freopen_s(&stream, "CONOUT$", "w", stdout);
	freopen_s(&stream, "CONOUT$", "w", stderr);

	HWND consoleHandle = GetConsoleWindow();
	HMENU hmenu = GetSystemMenu(consoleHandle, FALSE);
	EnableMenuItem(hmenu, SC_CLOSE, MF_GRAYED);
}

LRESULT Tool::ProcessMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DWORD mouseButton = 0;

	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_MENUCHAR: 
		return MAKELRESULT(0, MNC_CLOSE);

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_SIZE:
		graphics->Resize(LOWORD(lParam), HIWORD(lParam));

		return 0;

	case WM_LBUTTONDOWN:
		if (mouseButton == 0) mouseButton = VK_LBUTTON;
	case WM_MBUTTONDOWN:
		if (mouseButton == 0) mouseButton = VK_MBUTTON;
	case WM_RBUTTONDOWN:
		if (mouseButton == 0) mouseButton = VK_RBUTTON;
		Input::OnMouseDown(mouseButton, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), graphics->getHandle());
		return 0;

	case WM_LBUTTONUP:
		if (mouseButton == 0) mouseButton = VK_LBUTTON;
	case WM_MBUTTONUP:
		if (mouseButton == 0) mouseButton = VK_MBUTTON;
	case WM_RBUTTONUP:
		if (mouseButton == 0) mouseButton = VK_RBUTTON;
		Input::OnMouseUp(mouseButton, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_MOUSEMOVE:
		Input::OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_MOUSEWHEEL: // add to input
		Input::OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA);
		return 0;

	case WM_KEYDOWN:
		Input::OnKeyDown(wParam);
		return 0;

	case WM_KEYUP:
		Input::OnKeyUp(wParam);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
