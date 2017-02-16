#include <sstream>

#include "dear imgui\imgui.h"
#include "dear imgui\imgui_impl_dx11.h"

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

Tool::Tool()
{
	DXCoreInstance = this;
	resizing = false;

#if defined(DEBUG) || defined(_DEBUG)
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif
}

Tool::~Tool()
{
	ImGui_ImplDX11_Shutdown();
	delete graphics;
	delete camera;
	TransformData::ptr->ShutDown();
}

HRESULT Tool::Run(HINSTANCE hInstance, unsigned int windowWidth, unsigned int windowHeight)
{
	TransformData::Init(400);

	graphics = new Graphics(windowWidth, windowHeight);
	camera = new Camera();

	HRESULT hr = graphics->Init(hInstance);
	if (FAILED(hr)) return hr;

	ImGui_ImplDX11_Init(graphics->getHandle(), graphics->getDevice(), graphics->getContext());

	Input::bindToControl("Quit", VK_ESCAPE);

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
			ImGui_ImplDX11_NewFrame();

			float delta = ImGui::GetIO().DeltaTime;
			Update(delta);
			camera->Update(delta);
			graphics->Draw(camera, delta);

			Input::updateControlStates();
			
			ImGui::Render();
			graphics->Present();
		}
	}

	return (HRESULT)msg.wParam;
}

void Tool::Update(float deltaTime)
{
	if (Input::wasControlPressed("Quit"))
		Quit();

	//if (Input::isControlDown("camLook"))
	//{
	//	POINT pos;
	//	pos.x = graphics->GetWidth() / 2;
	//	pos.y = graphics->GetHeight() / 2;
	//	ClientToScreen(graphics->getHandle(), &pos);
	//	SetCursorPos(pos.x, pos.y);
	//}
	bool t;
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	if (!ImGui::Begin("Stats Bar", &t, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs))
	{
		ImGui::End();
		return;
	}
	ImGuiIO& io = ImGui::GetIO();
	ImGui::Text("Res: %.0fx%.0f\tFPS: %.0f Delta: %.5f", io.DisplaySize.x, io.DisplaySize.y, io.Framerate, io.DeltaTime * 1000);
	ImGui::End();
}

void Tool::Quit()
{
	PostQuitMessage(0);
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

void Tool::OnResize(unsigned int width, unsigned int height)
{
	ImGui_ImplDX11_InvalidateDeviceObjects();
	camera->Resize(width, height);
	graphics->Resize(width, height);
	ImGui_ImplDX11_CreateDeviceObjects();
}

extern LRESULT ImGui_ImplDX11_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT Tool::ProcessMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DWORD mouseButton = 0;
	static unsigned int width = 1280;
	static unsigned int height = 720;

	ImGui_ImplDX11_WndProcHandler(hWnd, uMsg, wParam, lParam);

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
		width = LOWORD(lParam);
		height = HIWORD(lParam);
		if (wParam == SIZE_MAXIMIZED || (wParam == SIZE_RESTORED && !resizing))
			OnResize(width, height);
		return 0;

	case WM_ENTERSIZEMOVE:
		resizing = true;
		return 0;

	case WM_EXITSIZEMOVE:
		resizing = false;
		OnResize(width, height);
		return 0;

	case WM_LBUTTONDOWN:
		if (mouseButton == 0) mouseButton = VK_LBUTTON;
	case WM_MBUTTONDOWN:
		if (mouseButton == 0) mouseButton = VK_MBUTTON;
	case WM_RBUTTONDOWN:
		if (mouseButton == 0) mouseButton = VK_RBUTTON;
		Input::OnMouseDown(mouseButton, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), graphics->getHandle());
		return 0;

	case WM_LBUTTONUP:
		if (mouseButton == 0) mouseButton = VK_LBUTTON;
	case WM_MBUTTONUP:
		if (mouseButton == 0) mouseButton = VK_MBUTTON;
	case WM_RBUTTONUP:
		if (mouseButton == 0) mouseButton = VK_RBUTTON;
		Input::OnMouseUp(mouseButton, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
		return 0;

	case WM_MOUSEMOVE:
		Input::OnMouseMove((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
		return 0;

	case WM_MOUSEWHEEL:
		Input::OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA);
		return 0;

	case WM_KEYDOWN:
		Input::OnKeyDown(wParam);
		return 0;

	case WM_KEYUP:
		Input::OnKeyUp(wParam);
		return 0;

	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
