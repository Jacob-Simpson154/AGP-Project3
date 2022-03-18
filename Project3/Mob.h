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
private:

};