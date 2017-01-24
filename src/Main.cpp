#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#include "WindowsPlatform.h"
#include <d3d11.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
// --------------------------------------------------------
// Entry point for a graphical (non-console) Windows application
// --------------------------------------------------------
int WINAPI WinMain(
	HINSTANCE hInstance, // The handle to this app's instance
	HINSTANCE hPrevInstance, // A handle to the previous instance of the app (always NULL)
	LPSTR lpCmdLine, // Command line params
	int nCmdShow) // How the window should be shown (we ignore this)
{
	BlackMagic::WindowsPlatform platform(hInstance);
	platform.BlackMagicInit();
	platform.BlackMagicCleanup();

	return 0;
}
#endif