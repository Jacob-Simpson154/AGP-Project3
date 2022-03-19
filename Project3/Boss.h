#pragma once
#include "Enemy.h"

class Boss : public Enemy
{
public: 
	Boss();
	~Boss();
	void Setup(RenderItem* geo, Camera* player);
	void Enemy::Movement();
	void Pattern_1();
	//void Enemy::Setup(int stage, int health, RenderItem* geo);
	/*void UpdateMovement();*/
private:
	/*int health = 100;
	int stage;
	RenderItem* geoObject;
	float tt = 0;*/
};