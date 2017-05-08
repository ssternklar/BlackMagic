#if defined(BM_PLATFORM_WINDOWS)
#include <d3d11.h>
#include <Windows.h>
#include "WindowsPlatform.h"
#endif

#include "TestGame.h"

#include "SceneBasedGame.h"
#include "RacingScene.h"

#if defined(BM_PLATFORM_WINDOWS)
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
	//TestGame* game = new TestGame;
	SceneBasedGame<RacingScene>* game = new SceneBasedGame<RacingScene>("scenes/title.scene");
	game->RunGame();
	delete game;
	platform.BlackMagicCleanup();

	return 0;
}
#endif