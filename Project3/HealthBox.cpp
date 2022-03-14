#include "HealthBox.h"

HealthBox::HealthBox()
{

}

HealthBox::HealthBox(float t)
{
	healthPercentageToRestore = t;
	timer = TimerInterval(30.0f);
}

HealthBox::~HealthBox()
{

}

float HealthBox::Consume()
{
	if (hasBeenConsumed == false)
	{
		hasBeenConsumed = true;
		timer.Start();
		return healthPercentageToRestore;
	}
	else return 0;
}

void HealthBox::Update(float gt)
{
	if (hasBeenConsumed == true)
	{
		if (timer.UpdateTimer(gt) == true)
		{
			hasBeenConsumed = false;
		}
	}
}


