#include <Windows.h>
#include "Game.h"

int WINAPI WinMain(HINSTANCE hInstance,	HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#ifdef _DEBUG
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	
	Game dxGame(hInstance);

	HRESULT hr = S_OK;
	hr = dxGame.InitWindow();
	if(FAILED(hr)) return hr;
	hr = dxGame.InitDirectX();
	if(FAILED(hr)) return hr;
	
	return dxGame.Run();
}
