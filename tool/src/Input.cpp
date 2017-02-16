#include "Input.h"

#include <map>
#include <vector>
#include <algorithm>

struct control
{
	std::vector<DWORD> bindings;
	bool isDown;
	bool wasDown;
};

static std::map<std::string, control> controls;
static DirectX::XMFLOAT2 prevMousePos = DirectX::XMFLOAT2(-1, -1);
static DirectX::XMFLOAT2 mousePos;
static float mouseScroll = 0;

namespace Input
{
	void bindToControl(std::string name, DWORD input)
	{
		auto controlCheck = controls.find(name);
		if (controlCheck == controls.end())
		{
			control newControl = { std::vector<DWORD>(), false, false };
			newControl.bindings.push_back(input);
			controls.insert(std::pair<std::string, control>(name, newControl));
			return;
		}

		auto bindCheck = std::find(controls[name].bindings.begin(), controls[name].bindings.end(), input);
		if (bindCheck == controls[name].bindings.end())
			controls[name].bindings.push_back(input);
	}

	void updateControlStates()
	{
		auto controlBucket = controls.begin();
		while (controlBucket != controls.end())
		{
			controlBucket->second.wasDown = controlBucket->second.isDown;
			controlBucket++;
		}
		prevMousePos = mousePos;
		mouseScroll = 0;
	}

	bool isControlDown(std::string name)
	{
		auto controlCheck = controls.find(name);
		if (controlCheck != controls.end())
			return controlCheck->second.isDown;
		return false;
	}

	bool isControlUp(std::string name)
	{
		auto controlCheck = controls.find(name);
		if (controlCheck != controls.end())
			return !controlCheck->second.isDown;
		return false;
	}

	bool wasControlPressed(std::string name)
	{
		auto controlCheck = controls.find(name);
		if (controlCheck != controls.end())
			return controlCheck->second.isDown && !controlCheck->second.wasDown;
		return false;
	}

	bool wasControlReleased(std::string name)
	{
		auto controlCheck = controls.find(name);
		if (controlCheck != controls.end())
			return !controlCheck->second.isDown && controlCheck->second.wasDown;
		return false;
	}

	void OnMouseDown(DWORD btn, int x, int y, HWND hMainWnd)
	{
		mousePos.x = (float)x;
		mousePos.y = (float)y;
		SetCapture(hMainWnd);

		auto controlBucket = controls.begin();
		while (controlBucket != controls.end())
		{
			for (size_t i = 0; i < controlBucket->second.bindings.size(); ++i)
				if (btn == controlBucket->second.bindings[i])
				{
					controlBucket->second.isDown = true;
					break;
				}
			controlBucket++;
		}
	}

	void OnMouseUp(DWORD btn, int x, int y)
	{
		auto controlBucket = controls.begin();
		while (controlBucket != controls.end())
		{
			for (size_t i = 0; i < controlBucket->second.bindings.size(); ++i)
				if (btn == controlBucket->second.bindings[i])
				{
					controlBucket->second.isDown = false;
					break;
				}
			controlBucket++;
		}

		ReleaseCapture();
	}

	void OnMouseMove(int x, int y)
	{
		mousePos.x = (float)x;
		mousePos.y = (float)y;
		if (prevMousePos.x == -1)
			prevMousePos = mousePos;
	}

	void OnMouseWheel(float scrollAmount)
	{
		mouseScroll = scrollAmount;
	}

	void OnKeyDown(WPARAM btn)
	{
		auto controlBucket = controls.begin();
		while (controlBucket != controls.end())
		{
			for (size_t i = 0; i < controlBucket->second.bindings.size(); ++i)
				if (btn == controlBucket->second.bindings[i])
				{
					controlBucket->second.isDown = true;
					break;
				}
			controlBucket++;
		}
	}

	void OnKeyUp(WPARAM btn)
	{
		auto controlBucket = controls.begin();
		while (controlBucket != controls.end())
		{
			for (size_t i = 0; i < controlBucket->second.bindings.size(); ++i)
				if (btn == controlBucket->second.bindings[i])
				{
					controlBucket->second.isDown = false;
					break;
				}
			controlBucket++;
		}
	}

	DirectX::XMFLOAT2 mouseOffset()
	{
		DirectX::XMVECTOR pos = XMLoadFloat2(&mousePos);
		DirectX::XMVECTOR ppos = XMLoadFloat2(&prevMousePos);
		DirectX::XMVECTOR offsetVec = DirectX::XMVectorSubtract(pos, ppos);
		DirectX::XMFLOAT2 offset;
		DirectX::XMStoreFloat2(&offset, offsetVec);

		return offset;
	}

	float mouseWheel()
	{
		return mouseScroll;
	}
}