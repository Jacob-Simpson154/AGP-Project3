#pragma once
//#include "RenderItemStruct.h"
#include "Enemy.h"

class Boss : public Enemy
{
public: 
	Boss();
	~Boss();
	void Setup(int s, int h, RenderItem* geo);
	void Enemy::Movement();
	/*void UpdateMovement();*/
private:
	int health = 100;
	int stage;
	RenderItem* geoObject;
	float tt = 0;
};