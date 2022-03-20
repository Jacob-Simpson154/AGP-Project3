#include "SpeedPowerup.h"

SpeedPowerup::SpeedPowerup()
{

}

SpeedPowerup::SpeedPowerup(float t, float m)
{
	maxTimeActive = t;
	speedMultiplier = m;
	timer = TimerInterval(150.0f);
}

SpeedPowerup::~SpeedPowerup()
{

}

float SpeedPowerup::Consume() //Change this to a vec2 or an array of floats as we need to send two int values through
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

void SpeedPowerup::Update(float gt)
{
	if (hasBeenConsumed == true)
	{
		if (timer.UpdateTimer(gt) == true)
		{
			hasBeenConsumed = false;
		}
	}
}


