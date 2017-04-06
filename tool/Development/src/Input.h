#pragma once

#include <string>
#include <Windows.h>
#include <DirectXMath.h>

namespace Input
{
	void BindToControl(std::string name, DWORD input);
	
	bool IsControlDown(std::string name);
	bool IsControlUp(std::string name);
	bool WasControlPressed(std::string name);
	bool WasControlReleased(std::string name);

	void UpdateControlStates();

	void OnKeyDown(WPARAM btn);
	void OnKeyUp(WPARAM btn);
	void OnMouseMove(int x, int y);
	DirectX::XMFLOAT2 getMouseDelta();
}