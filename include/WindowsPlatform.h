#pragma once
#include "PlatformBase.h"
#include "DX11Renderer.h"
#ifdef _WIN32
#include <Windows.h>
#include <windowsx.h>
#endif
namespace BlackMagic
{
	class WindowsPlatform : public PlatformBase
	{
		static WindowsPlatform* singletonRef;
		HINSTANCE hInstance;
		HWND hWnd;
		MSG msg = {};
		double perfCounterSeconds;
		float deltaTime;
		__int64 currentTime;
		__int64 previousTime;
		void HandleMouseMovement(WPARAM param, int x, int y);
	public:
		virtual bool InitWindow() override;
		virtual void InitPlatformAudioManager() override;
		virtual void InitPlatformThreadManager() override;
		virtual void InitPlatformRenderer() override;
		virtual bool GetSystemMemory(size_t size, byte** ptr) override;
		virtual void InputUpdate() override;
		virtual bool ShouldExit() override;
		virtual float GetDeltaTime() override;
		virtual void ReturnSystemMemory(byte* memory) override;
		static LRESULT CALLBACK WindowProc(
			HWND hWnd, // Window handle
			UINT uMsg, // Message
			WPARAM wParam, // Message's first parameter
			LPARAM lParam // Message's second parameter
		);
		WindowsPlatform(HINSTANCE instance);
		~WindowsPlatform();
		HINSTANCE GetHINSTANCE();
		HWND GetHWND();
		MSG GetMSG();
	};
}