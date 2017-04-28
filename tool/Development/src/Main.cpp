#include <Windows.h>
#include "Tool.h"

int WINAPI WinMain(HINSTANCE hInstance,	HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#ifdef _DEBUG
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	
	Tool tool;
	return tool.Run(hInstance, 1280, 720);

	//FILE* makeFile;
	//fopen_s(&makeFile, "defaultMaterial.mat", "wb");
	//if (!makeFile)
	//	return 1;
	//
	//size_t data[3] = {0, 0, 0};
	//
	//fwrite(data, sizeof(size_t), 3, makeFile);
	//
	//fclose(makeFile);
	//
	//return 0;
}
