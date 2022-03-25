#include "ShieldPowerup.h"

ShieldPowerup::ShieldPowerup()
{

}

ShieldPowerup::ShieldPowerup(float t)
{
	maxTimeActive = t;
	timer = TimerInterval(150.0f);
}

ShieldPowerup::~ShieldPowerup()
{

}

float ShieldPowerup::Consume()
{
	if (hasBeenConsumed == false)
	{
		hasBeenConsumed = true;
		timer.Start();
		return maxTimeActive;
	}
	else return 0;
}

void ShieldPowerup::Update(float gt)
{
	if (hasBeenConsumed == true)
	{
		if (timer.UpdateTimer(gt) == true)
		{
			hasBeenConsumed = false;
		}
	}
}


