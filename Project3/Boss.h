#pragma once
#include "RenderItemStruct.h"

class Boss
{
public: 
	Boss();
	~Boss();
	void Setup(int s, int h, RenderItem* geo);
	bool DealDamage(int d);
	void UpdateMovement();
private:
	int health = 100;
	int stage;
	RenderItem* geoObject;
	float tt = 0;
};