#include "QuadPowerup.h"

QuadPowerup::QuadPowerup()
{

}

QuadPowerup::QuadPowerup(float t, float m)
{
	maxTimeActive = t;
	damageMultiplier = m;
	timer = TimerInterval(150.0f);
}

QuadPowerup::~QuadPowerup()
{

}

float QuadPowerup::Consume() //Change this to a vec2 or an array of floats as we need to send two int values through
{
	if (hasBeenConsumed == false)
	{
		hasBeenConsumed = true;
		timer.Start();
		return maxTimeActive;
		//return speedMultiplier;
	}
	else return 0;
}

void QuadPowerup::Update(float gt)
{
	if (hasBeenConsumed == true)
	{
		if (timer.UpdateTimer(gt) == true)
		{
			hasBeenConsumed = false;
		}
	}
}


