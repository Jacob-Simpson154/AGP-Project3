#pragma once
#include "SimpleMath.h"
#include "Constants.h"


class Animation
{
	int currentFrame = 0;
	// updates frame when  = 0
	float cdTimer = 0.0f;
	const NoramlisedAnimData* pData;
public:
	// todo remove
	Animation() {};
	Animation(const NoramlisedAnimData* pAnimData);

	void Update(float dt);




};

