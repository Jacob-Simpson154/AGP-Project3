#pragma once
#include "Enemy.h"

class Mob : public Enemy
{
public:
	Mob();
	~Mob();
	void Setup(RenderItem* geo, Camera* player);
	void Enemy::Movement();
	//void Enemy::Setup(int stage, int health, RenderItem* geo);
	/*void UpdateMovement();*/
private:
	/*int health;
	int stage;
	RenderItem* geoObject;
	float tt = 0;*/
};