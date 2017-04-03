#include "allocators\globals.h"
#pragma once
namespace BlackMagic
{
	class InputData
	{
	public:
		enum Axis
		{
			X = 0,
			Y = 1
		};

		float axes[4];
		byte buttons[2];
		bool GetButton(int buttonID);
		void SetButton(int buttonID, bool on);
		float GetAxis(Axis axisID);
		void SetAxis(Axis axisID, float axis);
	};
}