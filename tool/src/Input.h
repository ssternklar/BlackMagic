#pragma once

#include <string>
#include <Windows.h>
#include <DirectXMath.h>

namespace Input
{
	void bindToControl(std::string name, DWORD input);
	bool isControlDown(std::string name);
	bool isControlUp(std::string name);
	bool wasControlPressed(std::string name);
	bool wasControlReleased(std::string name);
	void updateControlStates();
	void OnMouseDown(DWORD btn, int x, int y, HWND hMainWnd);
	void OnMouseUp(DWORD btn, int x, int y);
	void OnMouseMove(int x, int y);
	void OnMouseWheel(float scrollAmount);
	void OnKeyDown(WPARAM btn);
	void OnKeyUp(WPARAM btn);
	DirectX::XMFLOAT2 mouseOffset();
	float mouseWheel();
}