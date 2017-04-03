#include "InputData.h"

using namespace BlackMagic;

bool InputData::GetButton(int buttonID)
{
	int bit = buttonID % sizeof(byte);
	int index = buttonID / sizeof(byte);
	return ((buttons[index]) & 1 << bit) != 0;
}

void InputData::SetButton(int buttonID, bool on)
{
	int bit = buttonID % sizeof(byte);
	int index = buttonID / sizeof(byte);
	if (on)
	{
		buttons[index] |= 1 << bit;
	}
	else
	{
		buttons[index] &= (~(1 << bit));
	}
}

float InputData::GetAxis(InputData::Axis axisID)
{
	return axes[axisID];
}

void InputData::SetAxis(InputData::Axis axisID, float axis)
{
	axes[axisID] = axis;
}