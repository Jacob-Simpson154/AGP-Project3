#include "Animation.h"
#include "Constants.h"
void Animation::Update(float dt)
{
	if (pData->animating)
	{
		if (currentFrame != pData->numFrames || pData->loop)
		{
			cdTimer -= dt;
		}


		if (cdTimer <= 0.0f)
		{
			currentFrame = currentFrame % pData->numFrames;
			cdTimer = gc::ANIM_FRAME_TIME;
		}
	}




}