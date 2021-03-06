#include "WindowsPlatform.h"
#include "StdThreadManager.h"
#include "DirectXAudioManager.h"
#include <fstream>
using namespace BlackMagic;

bool WindowsPlatform::GetSystemMemory(size_t size, BlackMagic::byte** ptr)
{
	(*ptr = (BlackMagic::byte*)(::operator new(size)));
	bool ret = *ptr != nullptr;
	if (ret)
	{
		memset(*ptr, 0, size);
	}
	return ret;
}

void WindowsPlatform::HandleMouseMovement(WPARAM param, int x, int y)
{
	currentMousePos.x = ((float)x / windowWidth) * 2 - 1;
	currentMousePos.y = ((float)y / windowHeight) * 2 - 1;
	

}

LRESULT BlackMagic::WindowsPlatform::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Check the incoming message and handle any we care about
	switch (uMsg)
	{
		// This is the message that signifies the window closing
	case WM_DESTROY:
		PostQuitMessage(0); // Send a quit message to our own program
		return 0;

		// Prevent beeping when we "alt-enter"
	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);

		// Prevent the overall window from becoming too small
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

		// Sent when the window size changes
	case WM_SIZE:
		// Save the new client area dimensions.
		PlatformBase::GetSingleton()->SetScreenDimensions(LOWORD(lParam), HIWORD(lParam));

		return 0;

		// Mouse button being pressed (while the cursor is currently over our window)
	case WM_LBUTTONDOWN:
		PlatformBase::GetSingleton()->GetInputData()->SetButton(MouseButton::Left, true);
		SetCapture(hWnd);
		return 0;
	case WM_MBUTTONDOWN:
		PlatformBase::GetSingleton()->GetInputData()->SetButton(MouseButton::Middle, true);
		return 0;
	case WM_RBUTTONDOWN:
		PlatformBase::GetSingleton()->GetInputData()->SetButton(MouseButton::Right, true);
		//OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

		// Mouse button being released (while the cursor is currently over our window)
	case WM_LBUTTONUP:
		PlatformBase::GetSingleton()->GetInputData()->SetButton(MouseButton::Left, false);
		ReleaseCapture();
		return 0;
	case WM_MBUTTONUP:
		PlatformBase::GetSingleton()->GetInputData()->SetButton(MouseButton::Middle, false);
		return 0;
	case WM_RBUTTONUP:
		PlatformBase::GetSingleton()->GetInputData()->SetButton(MouseButton::Right, false);
		return 0;
		//OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		//return 0;

		// Cursor moves over the window (or outside, while we're currently capturing it)
	case WM_MOUSEMOVE:
		((WindowsPlatform*)PlatformBase::GetSingleton())->HandleMouseMovement(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		//OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

		// Mouse wheel is scrolled
	case WM_MOUSEWHEEL:
		//OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}

	// Let Windows handle any messages we're not touching
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void WindowsPlatform::InitPlatformRenderer()
{
	renderer = static_cast<Renderer*>(allocatorAllocator->allocate<DX11Renderer>());
	auto dxGDevice = new (renderer) DX11Renderer;
	dxGDevice->InitDx(hWnd, windowWidth, windowHeight);
	// The window exists but is not visible yet
	// We need to tell Windows to show it, and how to show it
	ShowWindow(hWnd, SW_SHOW);
}

void BlackMagic::WindowsPlatform::InitPlatformThreadManager()
{
	const size_t threadManagerWorkAreaSize = 1024 * 1024 * 64;
	byte* workArea = (byte*)allocatorAllocator->allocate(threadManagerWorkAreaSize);
	threadManager = static_cast<ThreadManager*>(allocatorAllocator->allocate<StdThreadManager>());
	threadManager = new (threadManager) StdThreadManager(this, workArea, threadManagerWorkAreaSize);
	threadManager->CreateGenericThread();
	threadManager->CreateContentThread();
	threadManager->CreateRenderThread();
	threadManager->CreateAudioThread();
}

bool WindowsPlatform::InitWindow()
{
	// Start window creation by filling out the
	// appropriate window class struct
	WNDCLASS wndClass = {}; // Zero out the memory
	wndClass.style = CS_HREDRAW | CS_VREDRAW; // Redraw on horizontal or vertical movement/adjustment
	wndClass.lpfnWndProc = WindowsPlatform::WindowProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance; // Our app's handle
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION); // Default icon
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW); // Default arrow cursor
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = "Direct3DWindowClass";

	// Attempt to register the window class we've defined
	if (!RegisterClass(&wndClass))
	{
		// Get the most recent error
		DWORD error = GetLastError();

		// If the class exists, that's actually fine.  Otherwise,
		// we can't proceed with the next step.
		if (error != ERROR_CLASS_ALREADY_EXISTS)
			return (bool)HRESULT_FROM_WIN32(error);
	}

	// Adjust the width and height so the "client size" matches
	// the width and height given (the inner-area of the window)
	RECT clientRect;
	SetRect(&clientRect, 0, 0, windowWidth, windowHeight);
	AdjustWindowRect(
		&clientRect,
		WS_OVERLAPPEDWINDOW, // Has a title bar, border, min and max buttons, etc.
		false); // No menu bar

				// Center the window to the screen
	RECT desktopRect;
	GetClientRect(GetDesktopWindow(), &desktopRect);
	int centeredX = (desktopRect.right / 2) - (clientRect.right / 2);
	int centeredY = (desktopRect.bottom / 2) - (clientRect.bottom / 2);

	// Actually ask Windows to create the window itself
	// using our settings so far.  This will return the
	// handle of the window, which we'll keep around for later
	hWnd = CreateWindow(
		wndClass.lpszClassName,
		"Black Magic Engine",
		WS_OVERLAPPEDWINDOW,
		centeredX,
		centeredY,
		clientRect.right - clientRect.left, // Calculated width
		clientRect.bottom - clientRect.top, // Calculated height
		0, // No parent window
		0, // No menu
		hInstance, // The app's handle
		0); // No other windows in our application

	__int64 perfFreq;
	QueryPerformanceFrequency((LARGE_INTEGER*)&perfFreq);
	perfCounterSeconds = 1.0 / (double)perfFreq;

	__int64 now;
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	currentTime = now;

	// Ensure the window was created properly
	if (hWnd == NULL)
	{
		return false;
		//DWORD error = GetLastError();
		//return HRESULT_FROM_WIN32(error);
	}

	// Return an "everything is ok" HRESULT value
	return true;
}

void BlackMagic::WindowsPlatform::InitPlatformAudioManager()
{
	audioManager = AllocateAndConstruct<DirectXAudioManager>(allocatorAllocator, 1);
	return;
}

#define KEYPRESSED(char) (GetAsyncKeyState(char) & 0x8000)

void WindowsPlatform::InputUpdate()
{
	// Determine if there is a message waiting
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		// Translate and dispatch the message
		// to our custom WindowProc function
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	//Needs to happen even if there's no mousemove message
	inputData.SetAxis(InputData::Axis::X, currentMousePos.x - lastMousePos.x);
	inputData.SetAxis(InputData::Axis::Y, currentMousePos.y - lastMousePos.y);
	lastMousePos.x = currentMousePos.x;
	lastMousePos.y = currentMousePos.y;

	//Some of this occurs in windowproc
	inputData.SetButton(Key::W, (bool)KEYPRESSED('W'));
	inputData.SetButton(Key::A, (bool)KEYPRESSED('A'));
	inputData.SetButton(Key::S, (bool)KEYPRESSED('S'));
	inputData.SetButton(Key::D, (bool)KEYPRESSED('D'));
	inputData.SetButton(Key::SPACE, (bool)KEYPRESSED(' '));
	inputData.SetButton(Key::ESCAPE, (bool)KEYPRESSED(VK_ESCAPE));
	inputData.SetButton(Key::SHIFT, (bool)KEYPRESSED(VK_SHIFT));
	inputData.SetButton(Key::LCTRL, (bool)KEYPRESSED(VK_LCONTROL));
}

bool BlackMagic::WindowsPlatform::ShouldExit()
{
	return manualExitTriggered || msg.message == WM_QUIT;
}

float BlackMagic::WindowsPlatform::GetDeltaTime()
{
	__int64 now;
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	currentTime = now;

	// Calculate delta time and clamp to zero
	//  - Could go negative if CPU goes into power save mode 
	//    or the process itself gets moved to another core
	deltaTime = max((float)((currentTime - previousTime) * perfCounterSeconds), 0.0f);

	// Save current time for next frame
	previousTime = currentTime;
	return deltaTime;
}

WindowsPlatform::WindowsPlatform(HINSTANCE instance)
{
	hInstance = instance;

#if defined(DEBUG) | defined(_DEBUG)
	// Enable memory leak detection as a quick and dirty
	// way of determining if we forgot to clean something up
	//  - You may want to use something more advanced, like Visual Leak Detector
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	
	//Open a console window
	CONSOLE_SCREEN_BUFFER_INFO info;

	AllocConsole();
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
	info.dwSize.X = 500;
	info.dwSize.Y = 120;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), info.dwSize);

	SMALL_RECT rect;
	rect.Left = 0;
	rect.Top = 0;
	rect.Right = 120;
	rect.Bottom = 32;

	FILE* f;
	freopen_s(&f, "CONIN$", "r", stdin);
	freopen_s(&f, "CONOUT$", "w", stdout);
	freopen_s(&f, "CONOUT$", "w", stderr);

	auto handle = GetConsoleWindow();
	auto menu = GetSystemMenu(handle, FALSE);
	EnableMenuItem(menu, SC_CLOSE, MF_GRAYED);
#endif

	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
	_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

}

void WindowsPlatform::ReturnSystemMemory(BlackMagic::byte* memory)
{
	if(memory)
	{
		delete[] memory;
	}
}

const char * BlackMagic::WindowsPlatform::GetAssetDirectory()
{
	return "./assets/";
}

bool BlackMagic::WindowsPlatform::ReadFileIntoMemory(const char* fileName, byte* fileBuffer, size_t bufferSize)
{
	std::ifstream file(fileName, std::ios::binary);
	if (file.is_open())
	{
		file.seekg(file.end);
		int fileLength = (int)file.tellg();
		file.seekg(file.beg);
#ifdef DEBUG
		if (fileLength > bufferSize)
		{
			printf("Incomplete file read");
		}
		if (bufferSize > fileLength)
		{
			printf("Buffer has extra characters in it");
		}
#endif
		file.read((char*)fileBuffer, bufferSize);
		return true;
	}
	return false;
}

unsigned int BlackMagic::WindowsPlatform::GetFileSize(const char* fileName)
{
	std::ifstream file(fileName, std::ios::binary | std::ios::ate);
	if(file.is_open())
	{
		unsigned int fileLength = (unsigned int)file.tellg();
		return fileLength;
	}
	return 0;
}

WindowsPlatform::~WindowsPlatform()
{
}

HINSTANCE BlackMagic::WindowsPlatform::GetHINSTANCE()
{
	return hInstance;
}

HWND BlackMagic::WindowsPlatform::GetHWND()
{
	return hWnd;
}

MSG BlackMagic::WindowsPlatform::GetMSG()
{
	return msg;
}

void WindowsPlatform::ShutdownPlatform()
{

}