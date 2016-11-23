#include "allocators\globals.h"
#pragma once
namespace BlackMagic
{
	class InputData
	{
	public:
		float axes[4];
		byte buttons[2];
		bool GetButton(int buttonID);
		void SetButton(int buttonID, bool on);
		float GetAxis(int axisID);
		void SetAxis(int axisID, float axis);
	};
}