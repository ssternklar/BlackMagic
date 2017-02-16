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
	void OnKeyDown(WPARAM btn);
	void OnKeyUp(WPARAM btn);
	void OnMouseMove(int x, int y);
	DirectX::XMFLOAT2 getMouseDelta();
}