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
static DirectX::XMFLOAT2 mouseDelta;

namespace Input
{
	void BindToControl(std::string name, DWORD input)
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

	void UpdateControlStates()
	{
		auto controlBucket = controls.begin();
		while (controlBucket != controls.end())
		{
			controlBucket->second.wasDown = controlBucket->second.isDown;
			controlBucket++;
		}
		mouseDelta.x = 0;
		mouseDelta.y = 0;
	}

	bool IsControlDown(std::string name)
	{
		auto controlCheck = controls.find(name);
		if (controlCheck != controls.end())
			return controlCheck->second.isDown;
		return false;
	}

	bool IsControlUp(std::string name)
	{
		auto controlCheck = controls.find(name);
		if (controlCheck != controls.end())
			return !controlCheck->second.isDown;
		return false;
	}

	bool WasControlPressed(std::string name)
	{
		auto controlCheck = controls.find(name);
		if (controlCheck != controls.end())
			return controlCheck->second.isDown && !controlCheck->second.wasDown;
		return false;
	}

	bool WasControlReleased(std::string name)
	{
		auto controlCheck = controls.find(name);
		if (controlCheck != controls.end())
			return !controlCheck->second.isDown && controlCheck->second.wasDown;
		return false;
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

	DirectX::XMFLOAT2 getMouseDelta()
	{
		return mouseDelta;
	}

	void OnMouseMove(int x, int y)
	{
		mouseDelta.x = (float)x;
		mouseDelta.y = (float)y;
	}
}