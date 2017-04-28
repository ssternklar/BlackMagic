#include "allocators\globals.h"
#pragma once
namespace BlackMagic
{
	struct MouseButton
	{
		enum
		{
			Left = 0,
			Middle = 1,
			Right = 2
		};
	};

	struct Key
	{
		enum
		{
			W = 3,
			A = 4,
			S = 5,
			D = 6,
			SHIFT = 7,
			SPACE = 8,
			ESCAPE = 9,
			LCTRL = 10,
		};
	};

	class InputData
	{
	public:
		enum Axis
		{
			X = 0,
			Y = 1
		};

		float axes[4];
		byte buttons[8];
		bool GetButton(int buttonID);
		void SetButton(int buttonID, bool on);
		float GetAxis(Axis axisID);
		void SetAxis(Axis axisID, float axis);
	};
}