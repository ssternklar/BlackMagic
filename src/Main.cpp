#include <Windows.h>
#include "Game.h"
#include "WindowsPlatform.h"
#include "FZERO.h"

// --------------------------------------------------------
// Entry point for a graphical (non-console) Windows application
// --------------------------------------------------------
int WINAPI WinMain(
	HINSTANCE hInstance, // The handle to this app's instance
	HINSTANCE hPrevInstance, // A handle to the previous instance of the app (always NULL)
	LPSTR lpCmdLine, // Command line params
	int nCmdShow) // How the window should be shown (we ignore this)
{
	//// Create the Game object using
	//// the app handle we got from WinMain
	//Game dxGame(hInstance);

	//// Result variable for function calls below
	//HRESULT hr = S_OK;

	//// Attempt to create the window for our program, and
	//// exit early if something failed
	//hr = dxGame.InitWindow();
	//if (FAILED(hr))
	//	return hr;

	//// Begin the message and game loop, and then return
	//// whatever we get back once the game loop is over
	//return dxGame.Run();

	BlackMagic::WindowsPlatform platform(hInstance);
	platform.BlackMagicInit();
	FZERO game(&platform);
	return game.RunGame();
}
