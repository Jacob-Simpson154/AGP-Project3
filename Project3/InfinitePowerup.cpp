#include "InfinitePowerup.h"

InfinitePowerup::InfinitePowerup()
{

}

InfinitePowerup::InfinitePowerup(float t)
{
	maxTimeActive = t;
	timer = TimerInterval(150.0f);
}

InfinitePowerup::~InfinitePowerup()
{

}

float InfinitePowerup::Consume()
{
	if (hasBeenConsumed == false)
	{
		hasBeenConsumed = true;
		timer.Start();
		return maxTimeActive;
	}
	else return 0;
}

void InfinitePowerup::Update(float gt)
{
	if (hasBeenConsumed == true)
	{
		if (timer.UpdateTimer(gt) == true)
		{
			hasBeenConsumed = false;
		}
	}
}


