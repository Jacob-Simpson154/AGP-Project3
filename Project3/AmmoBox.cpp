#include "AmmoBox.h"

AmmoBox::AmmoBox()
{

}

AmmoBox::AmmoBox(float t)
{
	ammoPercentageToRestore = t;
	timer = TimerInterval(30.0f);
}

AmmoBox::~AmmoBox()
{

}

float AmmoBox::Consume()
{
	if (hasBeenConsumed == false)
	{
		hasBeenConsumed = true;
		timer.Start();
		return ammoPercentageToRestore;
	}
	else return 0;
}

void AmmoBox::Update(float gt)
{
	if (hasBeenConsumed == true)
	{
		if (timer.UpdateTimer(gt) == true)
		{
			hasBeenConsumed = false;
		}
	}
}


